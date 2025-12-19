

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
typedef unsigned char uint8;
void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

#define MAX_ORDER 10   // Order to nhất: 2^10 * 4KB = 4MB
#define NBLOCKS (PHYSTOP / PGSIZE) // Tổng số trang 4KB trong RAM

struct {
  struct spinlock lock;
  
  struct run *freelists[MAX_ORDER + 1]; 


  uint8 split[NBLOCKS]; 
  

  uint8 orders[NBLOCKS]; 

} kmem;
int
block_index(void *pa)
{
  uint64 p = (uint64)pa;
  return (p - KERNBASE) / PGSIZE; 
}

void*
block_addr(int bi)
{
  return (void*)(KERNBASE + (bi * PGSIZE));
}

int
buddy_index(int bi, int order)
{
  return bi ^ (1 << order);
}
void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa.
void
kfree(void *pa)
{
  struct run *r;
  int bi, buddy_bi, order;

  // Kiểm tra địa chỉ hợp lệ
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Xóa rác bộ nhớ (optional)
  // memset(pa, 1, PGSIZE);

  bi = block_index(pa);
  order = kmem.orders[bi]; // Lấy kích thước hiện tại
  
  acquire(&kmem.lock);

  // --- LOGIC GỘP BUDDY (COALESCING) ---
  // Vòng lặp: Nếu Buddy rảnh thì gộp lại và tiếp tục leo lên order cao hơn
  while(order < MAX_ORDER){
    buddy_bi = buddy_index(bi, order); // Tìm vị trí Buddy

    // Kiểm tra: Buddy có đang nằm trong danh sách rảnh (freelist) không?
    struct run **ptr = &kmem.freelists[order];
    struct run *curr = *ptr;
    int found = 0;

    while(curr){
      if(block_index(curr) == buddy_bi){ // Tìm thấy Buddy đang rảnh!
        *ptr = curr->next; // Bắt nó ra khỏi hàng (unlink)
        found = 1;
        break; 
      }
      ptr = &curr->next;
      curr = *ptr;
    }

    if(found){
      // Nếu gộp được: Cập nhật chỉ số block về phía nhỏ hơn
      if(buddy_bi < bi) bi = buddy_bi;
      order++; // Tăng kích thước lên gấp đôi
    } else {
      break; // Buddy đang bận, không gộp được nữa
    }
  }

  // Cất khối (đã gộp to nhất có thể) vào danh sách
  r = (struct run*)block_addr(bi); 
  r->next = kmem.freelists[order]; 
  kmem.freelists[order] = r;

  // Cập nhật metadata
  kmem.orders[bi] = order;
  kmem.split[bi] = 0; 

  release(&kmem.lock);
}
// Allocate one 4096-byte page of physical memory.
void *
kalloc(void)
{
  struct run *r;
  int k;
  
  acquire(&kmem.lock);

  // Cần 1 trang (4KB) -> Order 0
  // Tìm từ order 0 lên order lớn nhất xem ai rảnh
  for(k = 0; k <= MAX_ORDER; k++){
    if(kmem.freelists[k]){
      r = kmem.freelists[k]; 
      kmem.freelists[k] = r->next; // Lấy ra
      
      // Nếu block lấy được to hơn 4KB (k > 0), phải CẮT NHỎ (Split)
      while(k > 0) {
        k--; // Giảm size xuống
        
        // Tính toán địa chỉ nửa sau (buddy) để cất đi
        int bi = block_index(r);
        int buddy_bi = buddy_index(bi, k); 
        struct run *buddy = (struct run*)block_addr(buddy_bi);

        // Cất nửa sau vào danh sách rảnh
        buddy->next = kmem.freelists[k];
        kmem.freelists[k] = buddy;

        // Đánh dấu kích thước
        kmem.orders[buddy_bi] = k;
        kmem.split[buddy_bi] = 0; // Buddy rảnh
        
        kmem.orders[bi] = k;
        kmem.split[bi] = 1; // Nửa đầu đang dùng/cắt tiếp
      }
      
      release(&kmem.lock);
      memset((char*)r, 5, PGSIZE); // Fill rác check lỗi
      return (void*)r;
    }
  }
  
  release(&kmem.lock);
  return 0; // Hết bộ nhớ (Out of Memory)
}
