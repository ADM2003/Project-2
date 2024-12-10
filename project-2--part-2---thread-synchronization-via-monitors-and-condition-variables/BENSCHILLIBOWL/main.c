#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "BENSCHILLIBOWL.h"

#define BENSCHILLIBOWL_SIZE 100
#define NUM_CUSTOMERS 90
#define NUM_COOKS 10
#define ORDERS_PER_CUSTOMER 3
#define EXPECTED_NUM_ORDERS NUM_CUSTOMERS * ORDERS_PER_CUSTOMER

// Global variable for the restaurant.
BENSCHILLIBOWL *bcb;

/**
 * Thread function that represents a customer. A customer should:
 *  - allocate space (memory) for an order.
 *  - select a menu item.
 *  - populate the order with their menu item and their customer ID.
 *  - add their order to the restaurant.
 */
void* BENSCHILLIBOWLCustomer(void* tid) {
    int customer_id = (int)(long)tid;
    int i;
    for (i = 0; i < ORDERS_PER_CUSTOMER; i++) {
        Order* order = (Order*)malloc(sizeof(Order));
        if (!order) {
            fprintf(stderr, "Failed to allocate memory for order.\n");
            continue;
        }
        order->menu_item = PickRandomMenuItem();
        order->customer_id = customer_id;
        int order_number = AddOrder(bcb, order); 
        printf("Customer #%d placed order %d: %s\n", customer_id, order_number, order->menu_item); 
    }
    return NULL;
}

/**
 * Thread function that represents a cook in the restaurant. A cook should:
 *  - get an order from the restaurant.
 *  - if the order is valid, it should fulfill the order, and then
 *    free the space taken by the order.
 * The cook should take orders from the restaurant until it does not
 * receive an order.
 */
void* BENSCHILLIBOWLCook(void* tid) {
    int cook_id = (int)(long)tid;

    while (1) {
        Order* order = GetOrder(bcb);  
        if (!order) break;    
        printf("Cook #%d fulfilled order #%d: %s\n", cook_id, order->order_number, order->menu_item);        
        free(order);                  
    }

    return NULL;
}

/**
 * Runs when the program begins executing. This program should:
 *  - open the restaurant
 *  - create customers and cooks
 *  - wait for all customers and cooks to be done
 *  - close the restaurant.
 */
int main() {
    pthread_t customer_threads[NUM_CUSTOMERS];
    pthread_t cook_threads[NUM_COOKS];

    bcb = OpenRestaurant(BENSCHILLIBOWL_SIZE, EXPECTED_NUM_ORDERS);
    long i;
    int j;

    for (i = 0; i < NUM_COOKS; i++) {
        pthread_create(&cook_threads[i], NULL, BENSCHILLIBOWLCook, (void*)i);
    }

    for (i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_create(&customer_threads[i], NULL, BENSCHILLIBOWLCustomer, (void*)i);
    }

    for (j = 0; j < NUM_CUSTOMERS; j++) {
        pthread_join(customer_threads[j], NULL);
    }

    pthread_mutex_lock(&bcb->mutex);
    bcb->all_orders_added = true;
    pthread_cond_broadcast(&bcb->can_get_orders);
    pthread_mutex_unlock(&bcb->mutex);

    for (j = 0; j < NUM_COOKS; j++) {
        pthread_join(cook_threads[j], NULL);
    }

    CloseRestaurant(bcb);

    return 0;
}
