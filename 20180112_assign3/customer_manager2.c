/* customer_manager2.c*/
/* Kim Seungsu, Assignment 3 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "customer_manager.h"

#define UNIT_BUCKET_SIZE 1024
#define MAX_BUCKET_SIZE 1048576
#define HASH_MULTIPLIER 65599

/* definition of struct UserInfo */
struct UserInfo {
	char *name;										// customer name		
	char *id;											// customer id
	int purchase;									// puchase amount (> 0)
	struct UserInfo *nextName;		// pointer to the next UserInfo node
	struct UserInfo *nextID;			// pointer to the next UserInfo node
};

/* definition of struct DB */
struct DB {
	struct UserInfo **nameTable;	// table using name as key
	struct UserInfo **idTable;		// table using id as key
	int curTableSize;							// current table size
	int numItems;									// # of sotred items needed to determine
																// whether the table should be expanded
};


/*--------------------------------------------------------------------*/
/* compute and return hash value with data with 'key' */
static int
hash_function(const char *pcKey, int iBucketCount)
{
	int i;
	unsigned int uiHash = 0U;
	for (i = 0; pcKey[i] != '\0'; i++) {
		uiHash = uiHash * (unsigned int)HASH_MULTIPLIER 
							+ (unsigned int)pcKey[i];
	}
	//return (int)(uiHash % (unsigned int)iBucketCount);
	return uiHash & (unsigned int)(iBucketCount -1);
}

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
	d->curTableSize = UNIT_BUCKET_SIZE;	// start with 1024 elements
	d->nameTable = (struct UserInfo **)calloc(d->curTableSize, 
					sizeof(struct UserInfo *));
	if (d->nameTable == NULL) {
		fprintf(stderr, "Can't allocate a memory for table of size %d\n"
			, d->curTableSize);
		free(d);
		return NULL;
	}
	d->idTable = (struct UserInfo **)calloc(d->curTableSize, 
				  sizeof(struct UserInfo *));
	if (d->idTable == NULL) {
		fprintf(stderr, "Can't allocate a memory for table of size %d\n"
			, d->curTableSize);
		free(d->nameTable);
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

	struct UserInfo *p;
	struct UserInfo *nextp;
	for (int i = 0; i < d->curTableSize; i++) {
		for (p = d->nameTable[i]; p != NULL; p = nextp) {
			nextp = p->nextName;
			free(p->name);
			free(p->id);
			free(p);
		}
	}
	free(d->nameTable);
	free(d->idTable);
	free(d);

	return;
}
/*--------------------------------------------------------------------*/
/* Register a customer with (name, id, purchaase) */
int
RegisterCustomer(DB_T d, const char *id,
	const char *name, const int purchase)
{
	// Check whether d or id or name is NULL, or purchase is zero
	// or negative
	assert(d && id && name);
	assert(purchase > 0);

	// Search is there a user info with the same name
	int appeared = 0;
	struct UserInfo *p;
	int hName = hash_function(name, d->curTableSize) % d->curTableSize;
	for (p = d->nameTable[hName]; p != NULL; p = p->nextName) {
		if (strcmp(p->name, name) == 0) {
			appeared = 1;
			break;
		}
	}
	
	// Search is there a user info with the same id
	int hID = hash_function(id, d->curTableSize) % d->curTableSize;
	if (!appeared) {
		for (p = d->idTable[hID]; p != NULL; p = p->nextID) {
			if (strcmp(p->id, id) == 0) {
				appeared = 1;
				break;
			}
		}
	}

	if (!appeared) {
		// If ther is no the same user info, mkae an user info
		// object with given information
		struct UserInfo *ui 
		= (struct UserInfo *)malloc(sizeof(struct UserInfo));
		if (ui == NULL) {
			fprintf(stderr, "Can't allocate a memory for UserInfo\n");
			return (-1);
		}
		ui->name = strdup(name);
		if (ui->name == NULL) {
			fprintf(stderr, "Can't allocate a memory for name\n");
			free(ui);
			return (-1);
		}
		ui->id = strdup(id);
		if (ui->name == NULL) {
			fprintf(stderr, "Can't allocate a memroy for id\n");
			free(ui->name);
			free(ui);
			return (-1);
		}
		ui->purchase = purchase;

		// If the number of Items in a hash table reaches 75% of
		// the number of buckets, expand the hash table
		int limit = 0.75 * d->curTableSize;
		if ((d->numItems >= limit) && 
			(d->curTableSize <= MAX_BUCKET_SIZE)) {
			struct UserInfo **newNameTable, **newIdTable;
			newNameTable = (struct UserInfo **)calloc(d->curTableSize * 2, 
				sizeof(struct UserInfo *));
			if (newNameTable == NULL) {
				fprintf(stderr, 
					"Can't allocate a memory for new hash table of size %d\n", 
					d->curTableSize * 2);
				return (-1);
			}

			newIdTable = (struct UserInfo **)calloc(d->curTableSize * 2,
				sizeof(struct UserInfo *));
			if (newIdTable == NULL) {
				fprintf(stderr,
					"Can't allocate a memory for new hash table of size %d\n",
					d->curTableSize * 2);
				free(newNameTable);
				return (-1);
			}

			struct UserInfo *p, *nextp;
			for (int i = 0; i < d->curTableSize; i++) {
				for (p = d->nameTable[i]; p != NULL; p = nextp) {
					nextp = p->nextName;
					hName = hash_function(p->name, d->curTableSize * 2)
					%(d->curTableSize * 2);
					hID = hash_function(p->id, d->curTableSize * 2)
					%(d->curTableSize * 2);
					p->nextName = newNameTable[hName];
					newNameTable[hName] = p;
					p->nextID = newIdTable[hID];
					newIdTable[hID] = p;
				}
			}

			free(d->nameTable);
			free(d->idTable);
			d->nameTable = newNameTable;
			d->idTable = newIdTable;
			d->curTableSize *= 2;
		}
		
		// Puts the user info into the DB
		hName = hash_function(name, d->curTableSize)%d->curTableSize;
		hID = hash_function(id, d->curTableSize)%d->curTableSize;

		ui->nextName = d->nameTable[hName];
		d->nameTable[hName] = ui;
		ui->nextID = d->idTable[hID];
		d->idTable[hID] = ui;

		d->numItems++;

		return 0;
	}

	// If there is the same user info in the DB, it's an error
	fprintf(stderr, "The same user info already exsits in the DB\n");
	return (-1);
}
/*--------------------------------------------------------------------*/
/* unregister a customer with 'id' */
int
UnregisterCustomerByID(DB_T d, const char *id)
{
	assert(d && id);

	struct UserInfo *p, *q, *prevp, *prevq;
	int hID = hash_function(id, d->curTableSize)%d->curTableSize;
	for (p = d->idTable[hID]; p != NULL; p = p->nextID) {
		if (strcmp(p->id, id) == 0) {
			if (p == d->idTable[hID]) {
				d->idTable[hID] = p->nextID;
			} else {
				prevp->nextID = p->nextID;
			}
			int hName = hash_function(p->name, d->curTableSize)
			%d->curTableSize;
			for (q = d->nameTable[hName]; q != NULL; q = q->nextName) {
				if (q == p) {
					if (q == d->nameTable[hName]) {
						d->nameTable[hName] = q->nextName;
					} else {
						prevq->nextName = q->nextName;
					}
				}
				prevq = q;
			}
			free(p->name);
			free(p->id);
			free(p);
			return 0;
		}
		prevp = p;
	}

	fprintf(stderr, "The user info with given id is not found\n");
	return (-1);
}
/*--------------------------------------------------------------------*/
/* unregister a customer with 'name' */
int
UnregisterCustomerByName(DB_T d, const char *name)
{
	assert(d && name);

	struct UserInfo *p, *q, *prevp, *prevq;
	int hName = hash_function(name, d->curTableSize)%d->curTableSize;
	for (p = d->nameTable[hName]; p != NULL; p = p->nextName) {
		if (strcmp(p->name, name) == 0) {
			if (p == d->nameTable[hName]) {
				d->nameTable[hName] = p->nextName;
			} else {
				prevp->nextName = p->nextName;
			}
			int hID = hash_function(p->id, d->curTableSize)%d->curTableSize;
			for (q = d->idTable[hID]; q != NULL; q = q->nextID) {
				if (q == p) {
					if (q == d->idTable[hID]) {
						d->idTable[hID] = q->nextID;
					} else {
						prevq->nextID = q->nextID;
					}
				}
				prevq = q;
			}
			free(p->name);
			free(p->id);
			free(p);
			return 0;
		}
		prevp = p;
	}

	fprintf(stderr, "The user info with given name is not found\n");
	return (-1);
}
/*--------------------------------------------------------------------*/
/* get the purchase amount of a user whose ID is 'id' */
int
GetPurchaseByID(DB_T d, const char* id)
{
	assert(d && id);

	struct UserInfo *p;
	int hID = hash_function(id, d->curTableSize)%d->curTableSize;
	for (p = d->idTable[hID]; p != NULL; p = p->nextID) {
		if (strcmp(p->id, id) == 0) {
			return p->purchase;
		}
	}

	return (-1);
}
/*--------------------------------------------------------------------*/
/* get the purchase amount of a user whose name is 'name' */
int
GetPurchaseByName(DB_T d, const char* name)
{
	assert(d && name);

	struct UserInfo *p;
	int hName = hash_function(name, d->curTableSize)%d->curTableSize;
	for (p = d->nameTable[hName]; p != NULL; p = p->nextName) {
		if (strcmp(p->name, name) == 0) {
			return p->purchase;
		}
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
	struct UserInfo *p;
	for (int i = 0; i < d->curTableSize; i++) {
		for (p = d->nameTable[i]; p != NULL; p = p->nextName) {
			sum += fp(p->id, p->name, p->purchase);
		}
	}

	return sum;
}