#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("--- Bat dau MEMTEST (Cong chua slay) ---\n");

  // Test 1: Cấp phát nhỏ lẻ
  printf("[Test 1] Cap phat nho le...\n");
  void *m1 = malloc(4096);
  if(m1 == 0) printf("That bai m1\n");
  else printf("Thanh cong m1\n");

  void *m2 = malloc(8192); // 2 trang
  if(m2 == 0) printf("That bai m2\n");
  else printf("Thanh cong m2\n");
  
  free(m1);
  free(m2);
  printf("[Test 1] Done.\n");

  // Test 2: Stress test - Ăn hết bộ nhớ
  printf("[Test 2] Stress test - An sach bo nho...\n");
  int count = 0;
  void *ptr[100]; 
  
  // Cố gắng cấp phát tối đa 100 block 4KB
  for(int i = 0; i < 100; i++){
    ptr[i] = malloc(4096);
    if(ptr[i] == 0){
      printf("Het bo nho o block thu %d (Chuyen nay binh thuong)\n", i);
      break;
    }
    count++;
  }
  
  printf("Da cap phat duoc %d blocks.\n", count);

  // Trả lại hết
  for(int i = 0; i < count; i++){
    free(ptr[i]);
  }
  printf("[Test 2] Da giai phong sach se.\n");

  printf("--- MEMTEST Hoan tat (Slay qua di!) ---\n");
  exit(0);
}
