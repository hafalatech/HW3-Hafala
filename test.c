#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "osqueue.h"
#include "threadPool.h"


/******************************************************************************/
/**************************[TASKS FUNCTIONS START]*****************************/
/******************************************************************************/

typedef struct awesomeContainer
{
   int awesomeNum;
   char* awesomeString;
}AwesomeContainer;

void awesomePrint(void* a)
{
   AwesomeContainer* con = (AwesomeContainer*)(a);
   int i;
   for(i=0 ; i<con->awesomeNum ; ++i)
   {
      printf("%s\n",con->awesomeString);
   }
}

void hello(void* a)
{
   printf("hello\n");
}

void doLongTask(void* a)
{
   long i;
   for (i=1; i != 0; i++)
   {
      ;
   }
   if (a==NULL)
   {
      return;
   }
   AwesomeContainer* con = (AwesomeContainer*)(a);
   con->awesomeNum = 1; // for success
}

void doMediumTask(void* a)
{
   printf("doing medium task\n");
   int j;
   for(j=0; j<1000; j++)
   {
      short i;
      for (i=1; i != 0; i++)
      {
         ;
      }
   }
   if (a==NULL)
   {
      return;
   }
   AwesomeContainer* con = (AwesomeContainer*)(a);
   con->awesomeNum = 1; // for success
}

void doMediumTaskNoPrint()
{
   int j;
   for(j=0; j<1000; j++)
   {
      short i;
      for (i=1; i != 0; i++)
      {
         ;
      }
   }
}

int fibonaciAux(int n)
{
   if (n < 2)
   {
      return n;
   }
   return fibonaciAux(n-1) + fibonaciAux(n-2);
}

void fibonaci(void* n)
{
   int num = (int)n;
   int res = fibonaciAux(num);
   printf("fibonaci of %d is %d\n",num,res);
}

void printingLongText(void* a)
{
   char* cannabisDraw = "                 0\n                00\n               0000\n   0          000000           0\n   00         000000           0\n    0000      000000          00\n    000000    0000000     00000\n 0     0000000 000000 00000000   0\n00      000000 00000 0000000    00\n0000     000000 000 000000    0000\n 000000000  0000 0 000 0 000000000\n    000000000  0 0 0 00000000000\n        000000000000000000000\n              000 0 0000\n            00000 0  00000\n          00       0       00\n                    0\n";
   char* line1 = "Cannabis is an annual, dioecious, flowering herb.";
   char* line2 = "The leaves are palmately compound or digitate, with serrate leaflets.";
   char* line3 = "The first pair of leaves usually have a single leaflet, the number gradually increasing up to a";
   char* line4 = "maximum of about thirteen leaflets per leaf (usually seven or nine), depending on variety and growing conditions.";
   
   printf("%s",cannabisDraw);
   doMediumTaskNoPrint();
   printf("%s\n",line1);
   doMediumTaskNoPrint();
   printf("%s\n",line2);
   doMediumTaskNoPrint();
   printf("%s\n",line3);
   doMediumTaskNoPrint();
   printf("%s\n",line4);  
}

/******************************************************************************/
/***************************[TASKS FUNCTIONS END]******************************/
/******************************************************************************/


void test_thread_pool_sanity()
{
   int i;
   
   ThreadPool* tp = tpCreate(3);
   for(i=0; i<3; ++i)
   {
      tpInsertTask(tp,hello,NULL);
   }
   
   tpDestroy(tp,1);
   printf("[OK]\n");
   printf(" \n");
}


void test_single_thread_many_tasks()
{
   ThreadPool* tp = tpCreate(1);

   tpInsertTask(tp,doMediumTask,NULL);
   tpInsertTask(tp,doMediumTask,NULL);
   tpInsertTask(tp,doMediumTask,NULL);
   tpInsertTask(tp,doMediumTask,NULL);
   int num1 = 10;
   tpInsertTask(tp,fibonaci,&num1);
   int num2 = 10;
   tpInsertTask(tp,fibonaci,&num2);
   int a=0;
   tpInsertTask(tp,doLongTask,&a);
   AwesomeContainer con;
   con.awesomeNum = 10;
   con.awesomeString = "Arnio";
   tpInsertTask(tp,awesomePrint,&con);
   tpInsertTask(tp,printingLongText,NULL);

   tpDestroy(tp,1);
   printf("[OK]\n");
   printf(" \n");
}


void test_destroy_should_not_wait_for_tasks()
{
   ThreadPool* tp = tpCreate(1);

   AwesomeContainer con;
   con.awesomeNum = 0;
   con.awesomeString = "DontCare"; // we use only the awesomeNum
   tpInsertTask(tp,doMediumTask,&con);
   tpInsertTask(tp,doMediumTask,&con);
   tpInsertTask(tp,doMediumTask,&con);

   

   tpDestroy(tp,0);
   assert(con.awesomeNum==1);
   printf("[OK]\n");
   printf(" \n");
}


void test_destroy_should_wait_for_tasks()
{
   ThreadPool* tp = tpCreate(10);

   AwesomeContainer con;
   con.awesomeNum = 0;
   con.awesomeString = "DontCare"; // we use only the awesomeNum
   tpInsertTask(tp,doLongTask,&con);

   tpDestroy(tp,1);


   assert(con.awesomeNum==1);
   printf("[OK]\n");
   printf(" \n");
}

// Once this operation is still taking place no concurrent tpDestroy() are allowed on the same threadPool
void test_destroy_twice()
{
   printf("[OK]\n");
   printf(" \n");
} 


void agressiveTest()
{
   //repeat the same test many times to check for rare cases
   printf("[OK]\n");
   printf(" \n");
}

int main()
{
   printf("Starting Test\n");
   printf("\n");
   

   printf("test_thread_pool_sanity... \n");
   test_thread_pool_sanity();


   printf("test_single_thread_many_tasks... \n");
   test_single_thread_many_tasks();

   // printf("test_destroy_should_not_wait_for_tasks... \n");
   // test_destroy_should_not_wait_for_tasks();

   // printf("test_destroy_should_wait_for_tasks... \n");
   // test_destroy_should_wait_for_tasks();


   printf("Ending Test\n");
   return 0;
}
