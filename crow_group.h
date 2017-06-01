//
// crow_group.h
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
#ifndef CROW_GROUP_H_INCLUDED
#define CROW_GROUP_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "libcrow.h"

crow_int_t	crow_group_measure					(crow_object_t* go, crow_byte_t layoutType);
void		crow_group_layout_changed			(crow_object_t* go, crow_byte_t layoutType);
void		crow_group_child_layout_changed		(crow_object_t* child, crow_byte_t layoutType);
void		crow_group_draw						(crow_object_t* go, cairo_t* ctx);
void		crow_group_update_cache				(crow_object_t* go, cairo_t* ctx);

#ifdef  __cplusplus
}
#endif
#endif
