#pragma once

template <typename T>
void freePointer(T * ptr)
{
  free(ptr);
  ptr = nullptr;
};
