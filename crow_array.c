//
// crow_array.c
//
// Author:
//       Jean-Philippe Bruyère <jp.bruyere@hotmail.com>
//
// Copyright (c) 2013-2017 Jean-Philippe Bruyère
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

//TODO: optimize array sizing with keeping enough space allocated preventing realloc calls
//but reduce if allocated spaces was a lot larger than needed.

#include <stdlib.h>

#include "crow_array.h"

#define ELASTICITY 10

crow_array_t* crow_array_create () {
	crow_array_t* arr = (crow_array_t*)malloc (sizeof (crow_array_t));
	arr->elements = (void **) malloc (0);
	arr->count = 0;
	arr->size = 0;
	return arr;
}

void crow_array_destroy (crow_array_t* arr) {
	free (arr->elements);
	free (arr);
}

void crow_array_add (crow_array_t* arr, void* element) {
	if (arr->size == arr->count){
		arr->size += ELASTICITY;
		arr->elements = (void **) realloc(arr->elements, (arr->size) * sizeof(void *));
	}
	arr->elements[arr->count] = element;
	arr->count++;
}

void crow_array_reset (crow_array_t* arr) {
	arr->size = 0;
	arr->count = 0;
	arr->elements = (void **) realloc(arr->elements, 0);
}