/* customer_manager1.c*/
/* Kim Seungsu, Assignment 3 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "customer_manager.h"

#define UNIT_ARRAY_SIZE 1024

/* definition of struct UserInfo */
struct UserInfo {
  char *name;                // customer name
  char *id;                  // customer id
  int purchase;              // purchase amount (> 0)
};

/* definition of struct DB */
struct DB {
  struct UserInfo *pArray;   // pointer to the array
  int curArrSize;            // current array size (max # of elements)
  int numItems;              // # of stored items, needed to determine
			     // # whether the array should be expanded
			     // # or not
};

/*--------------------------------------------------------------------*/
/* create and return a DB structure */
DB_T
CreateCustomerDB(void)
{
  DB_T d;
  
  d = (DB_T)calloc(1, sizeof(struct DB));
  if (d == NULL) {
    fprintf(stderr, "Can't allocate a memory for DB_T\n");
    return NULL;
  }
  d->curArrSize = UNIT_ARRAY_SIZE; // start with 1024 elements
  d->pArray = (struct UserInfo *)calloc(d->curArrSize,
               sizeof(struct UserInfo));
  if (d->pArray == NULL) {
    fprintf(stderr, "Can't allocate a memory for array of size %d\n",
	    d->curArrSize);   
    free(d);
    return NULL;
  }
  return d;
}
/*--------------------------------------------------------------------*/
/* Free DB structure and its associated memory */
void
DestroyCustomerDB(DB_T d)
{
  assert(d);

  struct UserInfo *p = d->pArray;
  for (int i = 0; i < d->numItems; i++) {
    free(p->name);
    free(p->id);
    p++;
  }
  free(d->pArray);
  free(d);

  return;
}
/*--------------------------------------------------------------------*/
/* Register a customer with (name, id, purchaase) */
int
RegisterCustomer(DB_T d, const char *id,
		 const char *name, const int purchase)
{
  assert(d && id && name);
  assert(purchase > 0);

  // Check if there's a user info with the same name or id in the DB.
  int appeared = 0;
  struct UserInfo *p = d->pArray;
  for (int i = 0; i < d->numItems; i++) {
    if (strcmp(p->name, name) == 0 || strcmp(p->id, id) == 0) {
      appeared = 1;
      break;
    }
    p++;
  }

  if (!appeared) {
    // If there's no the same user info, 
    //make an user info object with given information.
    struct UserInfo *ui = (struct UserInfo *)malloc(sizeof(struct UserInfo));
    if (ui == NULL) {
      fprintf(stderr, "Can't allocate a memory for UserInfo.\n");
      return (-1);
    }
    ui->name = strdup(name);
    if (ui->name == NULL) {
      fprintf(stderr, "Can't allocate a memory for string name.\n");
      free(ui);
      return (-1);
    }
    ui->id = strdup(id);
    if (ui->id == NULL) {
      fprintf(stderr, "Can't allocate a memory for string id.\n");
      free(ui->name);
      free(ui);
      return (-1);
    }
    ui->purchase = purchase;

    // If the DB is full, enlarge it.
    if (d->curArrSize <= d->numItems) {
      d->pArray = (struct UserInfo *)realloc(d->pArray, 
        d->curArrSize * sizeof(struct UserInfo) * 2);
      if (d->pArray == NULL) {
        fprintf(stderr,
          "Can't allocate a memory for new array of size %d.\n",
          d->curArrSize * 2);
        return (-1);
      }
      d->curArrSize *= 2;
    }

    // Puts the user info into the DB.
    p = d->pArray;
    for (int i = 0; i < d->numItems; i++) p++;
    *p = *ui;
    free(ui);
    d->numItems++;

    p = d->pArray;
    for (int i = 0; i < d->numItems; i++) {
      p++;
    }

    return 0;
  }

  // If there's same user info in the DB, it's an error.
  fprintf(stderr, "The same user info already exsits in the DB.\n");
  return (-1);
}
/*--------------------------------------------------------------------*/
/* unregister a customer with 'id' */
int
UnregisterCustomerByID(DB_T d, const char *id)
{  
  assert(d && id);

  int appeared = 0;
  struct UserInfo *p, *q;
  p = d->pArray;
  for (int i = 0; i < d->numItems; i++) {
    if (strcmp(p->id, id) == 0) {
      free(p->name);
      free(p->id);
      appeared = 1;
    }
    if (appeared && (i < d->numItems - 1)) {
      q = p;
      p++;
      *q = *p;
    } else {
      p++;
    }
  }

  if (!appeared) {
    fprintf(stderr, "The user info with given id is not found.\n");
    return (-1);
  }

  d->numItems--;
  return 0;
}

/*--------------------------------------------------------------------*/
/* unregister a customer with 'name' */
int
UnregisterCustomerByName(DB_T d, const char *name)
{
  assert(d && name);

  int appeared = 0;
  struct UserInfo *p, *q;
  p = d->pArray;
  for (int i = 0; i < d->numItems; i++) {
    if (strcmp(p->name, name) == 0) {
      free(p->name);
      free(p->id);
      appeared = 1;
    }
    if (appeared && (i < d->numItems - 1)) {
      q = p;
      p++;
      *q = *p;
    } else {
      p++;
    }
  }

  if (!appeared) {
    fprintf(stderr, "The user info with given id is not found.\n");
    return (-1);
  }

  d->numItems--;
  return 0;
}
/*--------------------------------------------------------------------*/
/* get the purchase amount of a user whose ID is 'id' */
int
GetPurchaseByID(DB_T d, const char* id)
{
  assert(d && id);

  struct UserInfo *p = d->pArray;
  for (int i = 0; i < d->numItems; i++) {
    if (strcmp(p->id, id) == 0) {
      return p->purchase;
    }
    p++;
  }

  return (-1);
}
/*--------------------------------------------------------------------*/
/* get the purchase amount of a user whose name is 'name' */
int
GetPurchaseByName(DB_T d, const char* name)
{
  assert(d && name);

  struct UserInfo *p = d->pArray;
  for (int i = 0; i < d->numItems; i++) {
    if (strcmp(p->name, name) == 0) {
      return p->purchase;
    }
    p++;
  }

  return (-1);
}
/*--------------------------------------------------------------------*/
/* iterate all valid user items once, evaluate fp for each valid user
   and return the sum of all fp function calls */
int
GetSumCustomerPurchase(DB_T d, FUNCPTR_T fp)
{
  if (!d || !fp) {
    fprintf(stderr, "d or fp is NULL\n");
    return (-1);
  }

  int sum = 0;
  struct UserInfo *p = d->pArray;
  for (int i = 0; i < d->numItems; i++) {
    sum += fp(p->id, p->name, p->purchase);
    p++;
  }

  return sum;
}
