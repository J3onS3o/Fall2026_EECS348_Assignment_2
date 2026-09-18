/*
 * Program:      EECS 348 Assignment 2 - CEO Email Priority Queue
 * Description:  Implements a MaxHeap-based priority queue (array/list
 *               based implementation, built entirely from scratch --
 *               no pre-existing heap libraries) that prioritizes a
 *               busy CEO's incoming emails. Emails are ranked first
 *               by sender category (Boss > Subordinate > Peer >
 *               ImportantPerson > OtherPerson) and, within the same
 *               category, by date (newest first).
 * Inputs:       Commands read line-by-line from standard input:
 *                 EMAIL <sender category>,<subject line>,<date>
 *                 NEXT
 *                 READ
 *                 COUNT
 * Output:       Terminal output for NEXT (sender/subject/date) and
 *               COUNT (remaining unread email count).
 * Collaborators: Generated code with ChatGPT and Gemini, Claude for helping to debug and optimize
 * Author:       Josselyn T. Bui
 * Created:      09-17-2026
 * Revised:      09-17-2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 512      // max length of one line of input
#define MAX_SUBJECT 256   // max length of a subject line
#define MAX_CATEGORY 32   // max length of a category string

/* ---------- Email record ---------- */
typedef struct {
    char category[MAX_CATEGORY]; // raw category string (Boss, Peer, etc.)
    char subject[MAX_SUBJECT];   // subject line text
    char date[11];                // date string MM-DD-YYYY, 10 chars + null
    int  priority;                 // numeric rank derived from category
    long dateValue;                // date encoded as YYYYMMDD for comparison
} Email;

/* ---------- List-based MaxHeap ---------- */
typedef struct {
    Email *data;     // dynamic array backing the heap (the "list")
    int size;          // current number of emails stored
    int capacity;      // allocated capacity of data[]
} MaxHeap;

/* Create and initialize an empty heap with a starting capacity. */
MaxHeap *heapCreate(int initialCapacity) {
    MaxHeap *h = (MaxHeap *)malloc(sizeof(MaxHeap));             // allocate heap struct
    h->data = (Email *)malloc(sizeof(Email) * initialCapacity);  // allocate array
    h->size = 0;                    // start empty
    h->capacity = initialCapacity;  // remember allocated size
    return h;
}

/* Double the array capacity when the heap runs out of room. */
void heapGrow(MaxHeap *h) {
    h->capacity *= 2;                                                  // double capacity
    h->data = (Email *)realloc(h->data, sizeof(Email) * h->capacity);  // resize array
}

/* Return 1 if email a has strictly higher priority than email b. */
int hasHigherPriority(Email a, Email b) {
    if (a.priority != b.priority) {         // different sender categories
        return a.priority > b.priority;     // bigger priority number wins
    }
    return a.dateValue > b.dateValue;       // same category: newer date wins
}

/* Swap two Email records (used while bubbling up/down the heap). */
void swapEmail(Email *a, Email *b) {
    Email temp = *a;  // hold a's value
    *a = *b;          // copy b into a
    *b = temp;        // copy held value into b
}

/* Restore heap order by moving a newly inserted item upward toward the root. */
void bubbleUp(MaxHeap *h, int index) {
    while (index > 0) {                        // stop once we reach the root
        int parent = (index - 1) / 2;          // index of parent node
        if (hasHigherPriority(h->data[index], h->data[parent])) {
            swapEmail(&h->data[index], &h->data[parent]);  // move item up
            index = parent;                                 // continue from parent
        } else {
            break;  // heap property already satisfied
        }
    }
}

/* Restore heap order by moving the root downward after a removal. */
void bubbleDown(MaxHeap *h, int index) {
    while (1) {
        int left = 2 * index + 1;    // left child index
        int right = 2 * index + 2;   // right child index
        int largest = index;         // assume current node is largest

        if (left < h->size && hasHigherPriority(h->data[left], h->data[largest]))
            largest = left;          // left child outranks current largest
        if (right < h->size && hasHigherPriority(h->data[right], h->data[largest]))
            largest = right;         // right child outranks current largest

        if (largest == index) break;                    // heap property restored
        swapEmail(&h->data[index], &h->data[largest]);   // move item down
        index = largest;                                  // continue from new spot
    }
}

/* Insert a new email into the heap, maintaining the max-heap property. */
void heapPush(MaxHeap *h, Email e) {
    if (h->size == h->capacity) heapGrow(h);  // ensure room to insert
    h->data[h->size] = e;                     // place at next free slot
    bubbleUp(h, h->size);                     // restore order upward
    h->size++;                                 // one more email stored
}

/* Look at the highest priority email without removing it. Returns 1 if found. */
int heapPeek(MaxHeap *h, Email *out) {
    if (h->size == 0) return 0;   // empty inbox, nothing to peek at
    *out = h->data[0];             // root always holds highest priority email
    return 1;
}

/* Remove the highest priority email from the heap. Returns 1 if one was removed. */
int heapPop(MaxHeap *h) {
    if (h->size == 0) return 0;               // nothing to remove
    h->data[0] = h->data[h->size - 1];        // move last element into root slot
    h->size--;                                  // shrink logical size by one
    bubbleDown(h, 0);                           // restore order downward
    return 1;
}

/* Convert a sender category string into its numeric priority rank. */
int categoryToPriority(const char *category) {
    if (strcmp(category, "Boss") == 0) return 5;              // read first
    if (strcmp(category, "Subordinate") == 0) return 4;       // read next
    if (strcmp(category, "Peer") == 0) return 3;               // read next
    if (strcmp(category, "ImportantPerson") == 0) return 2;   // read next
    if (strcmp(category, "OtherPerson") == 0) return 1;       // read last
    return 0;  // unrecognized category, treat as lowest priority
}

/* Convert an MM-DD-YYYY date string into a comparable YYYYMMDD integer. */
long dateToValue(const char *date) {
    int month, day, year;
    sscanf(date, "%d-%d-%d", &month, &day, &year);  // parse the three fields
    return (long)year * 10000 + month * 100 + day;   // larger number = later date
}

/* Remove the trailing newline/carriage-return left behind by fgets. */
void stripNewline(char *s) {
    size_t len = strlen(s);  // current string length
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0';  // chop off the newline character
        len--;               // recheck the new end of string
    }
}

/* Parse "EMAIL <category>,<subject>,<date>" and push it onto the heap. */
void handleEmailCommand(MaxHeap *h, char *line) {
    char *rest = line + 6;                  // skip past the "EMAIL " prefix
    char *category = strtok(rest, ",");     // first comma-delimited field
    char *subject  = strtok(NULL, ",");     // second field, may contain spaces
    char *date     = strtok(NULL, ",");     // third field, the date

    if (!category || !subject || !date) return;  // malformed line, ignore safely

    Email e;                                              // build the new record
    strncpy(e.category, category, MAX_CATEGORY - 1);
    e.category[MAX_CATEGORY - 1] = '\0';                  // guarantee null terminator
    strncpy(e.subject, subject, MAX_SUBJECT - 1);
    e.subject[MAX_SUBJECT - 1] = '\0';                    // guarantee null terminator
    strncpy(e.date, date, 10);
    e.date[10] = '\0';                                     // guarantee null terminator
    e.priority = categoryToPriority(e.category);           // rank by category
    e.dateValue = dateToValue(e.date);                      // rank by date

    heapPush(h, e);  // add the fully-built email record to the heap
}

/* Handle the NEXT command: show the top email without removing it. */
void handleNextCommand(MaxHeap *h) {
    Email top;
    if (!heapPeek(h, &top)) return;  // empty inbox, nothing to display
    printf("Next email:\n");
    printf("Sender: %s\n", top.category);
    printf("Subject: %s\n", top.subject);
    printf("Date: %s\n", top.date);
}

/* Handle the READ command: remove the top email; no output is required. */
void handleReadCommand(MaxHeap *h) {
    heapPop(h);  // discard the highest priority email, if one exists
}

/* Handle the COUNT command: report how many emails remain unread. */
void handleCountCommand(MaxHeap *h) {
    printf("There are %d emails to read.\n", h->size);
}

/* Free all heap-allocated memory before the program exits. */
void heapDestroy(MaxHeap *h) {
    free(h->data);  // free the backing array
    free(h);        // free the heap struct itself
}

int main(void) {
    MaxHeap *inbox = heapCreate(16);  // start with room for 16 emails
    char line[MAX_LINE];               // buffer for one line of input

    while (fgets(line, sizeof(line), stdin)) {  // read commands until EOF
        stripNewline(line);                      // clean up the line ending
        if (strncmp(line, "EMAIL ", 6) == 0) {
            handleEmailCommand(inbox, line);      // add a new email
        } else if (strcmp(line, "NEXT") == 0) {
            handleNextCommand(inbox);             // peek at the top email
        } else if (strcmp(line, "READ") == 0) {
            handleReadCommand(inbox);             // remove the top email
        } else if (strcmp(line, "COUNT") == 0) {
            handleCountCommand(inbox);            // report unread count
        }
        // any other or blank line is silently ignored
    }

    heapDestroy(inbox);  // free memory before exiting
    return 0;
}