
/**
 * @file app_manager.c
 */

/*********************
 *      INCLUDES
 *********************/

#include "app_manager.h"

#include <stdlib.h>
#include <string.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct app_node
{
    const app_info_t *info;
    struct app_node *next;
} app_node_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static int compare(const void *a, const void *b);

/**********************
 *  STATIC VARIABLES
 **********************/

static app_node_t *head = NULL;
static int app_count = 0;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void app_registry_register(const app_info_t *info)
{
    app_node_t *node = malloc(sizeof(app_node_t));
    if (node == NULL)
        return; /* Handle malloc failure */
    node->info = info;
    node->next = head;
    head = node;
}

void app_registry_iterate_unsorted(app_register_cb cb)
{
    app_node_t *current = head;
    app_count = 0;
    while (current != NULL)
    {
        const app_info_t *info = current->info;
        cb(info->name, info->icon, info->callback);
        current = current->next;
        app_count++;
    }
}

void app_registry_iterate(app_register_cb cb)
{
    /* Count the apps */
    int count = 0;
    app_node_t *current = head;
    while (current != NULL)
    {
        count++;
        current = current->next;
    }

    /* Copy pointers to array */
    if (count == 0)
        return;

    const app_info_t **array = malloc(count * sizeof(app_info_t *));
    if (!array)
        return;

    current = head;
    for (int i = 0; i < count; i++)
    {
        array[i] = current->info;
        current = current->next;
    }

    /* Sort the array by name */
    qsort(array, count, sizeof(app_info_t *), compare);

    app_count = 0;
    /* Call callback in sorted order */
    for (int i = 0; i < count; i++)
    {
        cb(array[i]->name, array[i]->icon, array[i]->callback);
        app_count++;
    }

    /* Cleanup */
    free(array);
}

int app_registry_get_count()
{
    return app_count;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static int compare(const void *a, const void *b)
{
    const app_info_t *appA = *(const app_info_t **)a;
    const app_info_t *appB = *(const app_info_t **)b;
    return strcmp(appA->name, appB->name);
}
