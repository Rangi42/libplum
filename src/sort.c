#include "proto.h"

void sort_values (uint64_t * restrict data, uint64_t count) {
  #define THRESHOLD 16
  uint64_t * buffer;
  if (count < THRESHOLD || !(buffer = malloc(count * sizeof *buffer))) {
    quicksort_values(data, count);
    return;
  }
  uint_fast64_t start = 0, runsize = 2;
  bool descending = data[1] < *data;
  for (uint_fast64_t current = 2; current < count; current ++)
    if (descending ? data[current] <= data[current - 1] : data[current] >= data[current - 1])
      runsize ++;
    else {
      if (descending && runsize >= THRESHOLD)
        for (uint_fast64_t p = 0; p < runsize / 2; p ++) swap(uint_fast64_t, data[start + p], data[start + runsize - 1 - p]);
      buffer[start] = runsize;
      start = current ++;
      if (current == count)
        runsize = 1;
      else {
        descending = data[current] < data[current - 1];
        runsize = 2;
      }
    }
  if (descending && runsize >= THRESHOLD)
    for (uint_fast64_t p = 0; p < runsize / 2; p ++) swap(uint_fast64_t, data[start + p], data[start + runsize - 1 - p]);
  buffer[start] = runsize;
  start = 0;
  for (uint_fast64_t current = 0; current < count; current += buffer[current])
    if (buffer[current] >= THRESHOLD) {
      if (start != current) quicksort_values(data + start, buffer[start] = current - start);
      start = current + buffer[current];
    }
  #undef THRESHOLD
  if (start != count) quicksort_values(data + start, buffer[start] = count - start);
  while (*buffer != count) {
    merge_sorted_values(data, count, buffer);
    merge_sorted_values(buffer, count, data);
  }
  free(buffer);
}

void quicksort_values (uint64_t * restrict data, uint64_t count) {
  switch (count) {
    case 3:
      if (*data > data[2]) swap(uint_fast64_t, *data, data[2]);
      if (data[1] > data[2]) swap(uint_fast64_t, data[1], data[2]);
    case 2:
      if (*data > data[1]) swap(uint_fast64_t, *data, data[1]);
    case 0: case 1:
      return;
  }
  uint_fast64_t pivot = data[count / 2], left = UINT_FAST64_MAX, right = count;
  while (true) {
    while (data[++ left] < pivot);
    while (data[-- right] > pivot);
    if (left >= right) break;
    swap(uint_fast64_t, data[left], data[right]);
  }
  right ++;
  if (right > 1) quicksort_values(data, right);
  if (count - right > 1) quicksort_values(data + right, count - right);
}

void merge_sorted_values (uint64_t * restrict data, uint64_t count, uint64_t * restrict runs) {
  // in: data = data to sort, runs = run lengths; out: flipped
  uint_fast64_t length;
  for (uint_fast64_t current = 0; current < count; current += length) {
    length = runs[current];
    if (current + length == count)
      memcpy(runs + current, data + current, length * sizeof *data);
    else {
      // index1, index2 point to the END of the respective runs
      uint_fast64_t remaining1 = length, index1 = current + remaining1, remaining2 = runs[index1], index2 = index1 + remaining2;
      length = remaining1 + remaining2;
      for (uint64_t p = 0; p < length; p ++)
        if (!remaining2 || (remaining1 && data[index1 - remaining1] <= data[index2 - remaining2]))
          runs[current + p] = data[index1 - (remaining1 --)];
        else
          runs[current + p] = data[index2 - (remaining2 --)];
    }
    data[current] = length;
  }
}

#define comparepairs(first, op, second) (((first).index == (second).index) ? ((first).value op (second).value) : ((first).index op (second).index))

void sort_pairs (struct pair * restrict data, uint64_t count) {
  // this function and its helpers implement essentially the same algorithm as above, but adapted for index/value pairs instead of just values
  #define THRESHOLD 16
  struct pair * buffer;
  if (count < THRESHOLD || !(buffer = malloc(count * sizeof *buffer))) {
    quicksort_pairs(data, count);
    return;
  }
  uint_fast64_t start = 0, runsize = 2;
  bool descending = comparepairs(data[1], <, *data);
  for (uint_fast64_t current = 2; current < count; current ++)
    if (descending ? comparepairs(data[current], <=, data[current - 1]) : comparepairs(data[current], >=, data[current - 1]))
      runsize ++;
    else {
      if (descending && runsize >= THRESHOLD)
        for (uint_fast64_t p = 0; p < runsize / 2; p ++) swap(struct pair, data[start + p], data[start + runsize - 1 - p]);
      buffer[start].index = runsize;
      start = current ++;
      if (current == count)
        runsize = 1;
      else {
        descending = comparepairs(data[current], <, data[current - 1]);
        runsize = 2;
      }
    }
  if (descending && runsize >= THRESHOLD)
    for (uint_fast64_t p = 0; p < runsize / 2; p ++) swap(struct pair, data[start + p], data[start + runsize - 1 - p]);
  buffer[start].index = runsize;
  start = 0;
  for (uint_fast64_t current = 0; current < count; current += buffer[current].index)
    if (buffer[current].index >= THRESHOLD) {
      if (start != current) quicksort_pairs(data + start, buffer[start].index = current - start);
      start = current + buffer[current].index;
    }
  #undef THRESHOLD
  if (start != count) quicksort_pairs(data + start, buffer[start].index = count - start);
  while (buffer -> index != count) {
    merge_sorted_pairs(data, count, buffer);
    merge_sorted_pairs(buffer, count, data);
  }
  free(buffer);
}

void quicksort_pairs (struct pair * restrict data, uint64_t count) {
  switch (count) {
    case 3:
      if (comparepairs(*data, >, data[2])) swap(struct pair, *data, data[2]);
      if (comparepairs(data[1], >, data[2])) swap(struct pair, data[1], data[2]);
    case 2:
      if (comparepairs(*data, >, data[1])) swap(struct pair, *data, data[1]);
    case 0: case 1:
      return;
  }
  struct pair pivot = data[count / 2];
  uint_fast64_t left = UINT_FAST64_MAX, right = count;
  while (true) {
    do left ++; while (comparepairs(data[left], <, pivot));
    do right --; while (comparepairs(data[right], >, pivot));
    if (left >= right) break;
    swap(struct pair, data[left], data[right]);
  }
  right ++;
  if (right > 1) quicksort_pairs(data, right);
  if (count - right > 1) quicksort_pairs(data + right, count - right);
}

void merge_sorted_pairs (struct pair * restrict data, uint64_t count, struct pair * restrict runs) {
  // in: data = data to sort, runs = run lengths; out: flipped
  uint_fast64_t length;
  for (uint_fast64_t current = 0; current < count; current += length) {
    length = runs[current].index;
    if (current + length == count)
      memcpy(runs + current, data + current, length * sizeof *data);
    else {
      // index1, index2 point to the END of the respective runs
      uint_fast64_t remaining1 = length, index1 = current + remaining1, remaining2 = runs[index1].index, index2 = index1 + remaining2;
      length = remaining1 + remaining2;
      for (uint64_t p = 0; p < length; p ++)
        if (!remaining2 || (remaining1 && comparepairs(data[index1 - remaining1], <=, data[index2 - remaining2])))
          runs[current + p] = data[index1 - (remaining1 --)];
        else
          runs[current + p] = data[index2 - (remaining2 --)];
    }
    data[current].index = length;
  }
}

#undef comparepairs
