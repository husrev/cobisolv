/*
 Copyright 2024 Peter Kreye

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#include "cobi.h"

#include "extern.h"  // qubo header file: global variable declarations
#include "macros.h"

// #include "extern.h"  // qubo header file: global variable declarations
// #include "macros.h"

#include <pigpio.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif


//
#define WEIGHT_2  2
#define WEIGHT_3  3
#define SCANOUT_CLK  4
#define SAMPLE_CLK  17
#define ALL_ROW_HI  27
#define WEIGHT_1  22
#define WEIGHT_EN  10
#define COL_ADDR_4  9
#define ADDR_EN64_CHIP1  11
#define COL_ADDR_3  5
#define COL_ADDR_1  6
#define COL_ADDR_0  13
#define ROW_ADDR_2  19
#define ROW_ADDR_3  26
#define WEIGHT_5  14
#define SCANOUT_DOUT64_CHIP2  15
#define SCANOUT_DOUT64_CHIP1  18
#define WEIGHT_0  23
#define ROSC_EN  24
#define COL_ADDR_5  25
#define ROW_ADDR_5  8
#define ADDR_EN64_CHIP2  7
#define WEIGHT_4  1
#define COL_ADDR_2  12
#define ROW_ADDR_1  16
#define ROW_ADDR_0  20
#define ROW_ADDR_4  21

const int ROW_ADDRS[6] = {
    ROW_ADDR_0, ROW_ADDR_1,
    ROW_ADDR_2, ROW_ADDR_3,
    ROW_ADDR_4, ROW_ADDR_5
};

const int COL_ADDRS[6] = {
    COL_ADDR_0, COL_ADDR_1,
    COL_ADDR_2, COL_ADDR_3,
    COL_ADDR_4, COL_ADDR_5
};

const int WEIGHTS[6] = {
    WEIGHT_0, WEIGHT_1,
    WEIGHT_2, WEIGHT_3,
    WEIGHT_4, WEIGHT_5
};

const int NUM_GROUPS = 59;
const int COBIFIXED65_BASEGROUPS[59] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61};

const int BLANK_GRAPH[64][64] = {
    {0,0,4,3,3,3,3,0,5,7,5,0,4,7,7,3,3,7,2,5,2,7,2,3,7,3,0,7,3,4,5,3,0,0,3,3,3,7,7,3,7,5,5,5,3,2,1,2,4,5,2,4,2,2,4,5,7,3,3,0,3,1,7,0},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7},
    {0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,3},
    {0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,3},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,4},
    {0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,3},
    {0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,5},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
    {0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,4},
    {0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,4},
    {0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,3},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,3},
    {0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,2},
    {0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,3},
    {0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
    {0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
    {0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
    {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
    {0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3},
    {0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
    {0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
    {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
    {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
    {0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3},
    {0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3},
    {0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
    {0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
    {0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
    {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
    {0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
    {0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
    {0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
    {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
    {0,2,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3},
    {0,1,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
    {0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
    {0,7,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
    {0,2,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
    {0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
    {0,2,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3},
    {0,1,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
    {0,4,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3},
    {0,2,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3},
    {0,4,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
    {0,0,4,4,4,2,4,7,4,7,7,6,3,7,7,4,4,7,4,4,1,7,7,7,7,4,2,7,3,5,4,3,1,1,2,4,7,7,7,3,7,5,4,5,3,7,7,2,5,5,1,3,3,3,3,4,7,5,3,4,3,4,7,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

// Misc utility functions

int usleep(long usecs)
{
   struct timespec rem;
   struct timespec req= {
       (int)(usecs / 1000000),
       (usecs % 1000000) * 1000
   };

   return nanosleep(&req , &rem);
}

int *_malloc_array1d(int len)
{
    int* a = (int*)malloc(sizeof(int *) * len);
    if (a == NULL) {
        fprintf(stderr, "Bad malloc %s %s:%d\n", __FUNCTION__, __FILE__, __LINE__);
        exit(1);
    }

    int i;
    for (i = 0; i < len; i++) {
        a[i] = 0;
    }

    return a;
}

double **_malloc_double_array2d(int w, int h)
{
    double** a = (double**)malloc(sizeof(double *) * w);
    if (a == NULL) {
        fprintf(stderr, "Bad malloc %s %s:%d\n", __FUNCTION__, __FILE__, __LINE__);
        exit(1);
    }

    int i, j;
    for (i = 0; i < w; i++) {
        a[i] = (double*)malloc(sizeof(double) * h);
        for (j = 0; j < h; j++) {
            a[i][j] = 0;
        }
    }

    return a;
}

int **_malloc_array2d(int w, int h)
{
    int** a = (int**)malloc(sizeof(int *) * w);
    if (a == NULL) {
        fprintf(stderr, "Bad malloc %s %s:%d\n", __FUNCTION__, __FILE__, __LINE__);
        exit(1);
    }

    int i, j;
    for (i = 0; i < w; i++) {
        a[i] = (int*)malloc(sizeof(int) * h);
        for (j = 0; j < h; j++) {
            a[i][j] = 0;
        }
    }

    return a;
}

void _free_array2d(void **a, int w) {
    int i;
    for (i = 0; i < w; i++) {
        free(a[i]);
    }
    free(a);
}

void binary_splice_rev(int num, int *bin_list)
{
    // given num place lower 6 bits, in reverse order, into bin_list
    int i;
    int shift = 0;
    for (i = 0; i < 6; i++) {
        bin_list[i] = (num >> shift) & 1;
        shift++;
    }
}

/* _outer_prod

   Assumes `result` has dimensions (`len_a`, `len_b`).
 */
void _outer_prod(int *a, size_t len_a, int *b, size_t len_b, int **result)
{
    size_t i, j;

    for (i = 0; i < len_a; i++) {
        for (j = 0; j < len_b; j++) {
            result[i][j] = a[i] * b[j];
        }
    }
}

void _scalar_mult(int *a, size_t len, int scalar)
{
    size_t i;
    for (i = 0; i < len; i++) {
        a[i] = a[i] * scalar;
    }
}

/* _transpose

   Returns newly allocated 2d array containing the result.
 */
int **_transpose(int **a, int w, int h)
{
    int **result = _malloc_array2d(h, w);

    int i, j;
    for (i = 0; i < w; i++) {
        for (j = 0; j < h; j++) {
            result[j][i] = a[i][j];
        }
    }

    return result;
}

/* Element-wise addition of 2 2D arrays. Result stored in first array. */
void _add_array2d(int **a, int **b, int w, int h)
{
    int i, j;
    for (i = 0; i < w; i++) {
        for (j = 0; j < h; j++) {
            a[i][j] += b[i][j];
        }
    }
}

int **_matrix_mult(int **a, int a_w, int a_h, int **b, int b_h)
{
    // b is assumed to have dimensions (a_h, b_h)
    // int b_w = a_h;

    int **result = _malloc_array2d(a_w, b_h);

    int i, j, k;
    for(i = 0; i < a_w; i++) { // each row of a
        for(j = 0; j < b_h; j++) { // each col in b
            result[i][j] = 0;
            for(k = 0; k < a_h; k++) { // each col a
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }

    return result;
}

// Elementwise multiplication of 2D arrays
int **_array2d_element_mult(int **a, int **b, int w, int h)
{
    int **result = _malloc_array2d(w, h);

    int i, j;
    for (i = 0; i < w; i++) {
        for (j = 0; j < h; j++) {
            result[i][j] = a[i][j] * b[i][j];
        }
    }

    return result;
}

void cobi_simple_descent(int *spins, int **weights)
{
    // Assumption: NUM_GROUPS == len(weights) == len(weights[i]) == len(spins)
    int size = NUM_GROUPS;

    int neg_spins[NUM_GROUPS];
    for (int i = 0; i < size; i++) {
        // _scalar_mult(neg_spins, size, -1);
        neg_spins[i] = spins[i] * -1;
    }

    int **cross = _malloc_array2d(size, size);
    // int cross[NUM_GROUPS][NUM_GROUPS]
    _outer_prod(spins, size, neg_spins, size, cross);

    // # prod = cross * (weights+weights.transpose())
    int **wt_transpose = _transpose(weights, size, size);
    // add weights to wt_transpose, storing result in wt_transpose
    _add_array2d(wt_transpose, weights, size, size);
    int **prod = _array2d_element_mult(cross, wt_transpose, size, size);

    // # diffs = np.sum(prod,0)
    int diffs[NUM_GROUPS];
    // Column-wise sum
    for (int i = 0; i < size; i++) {
        diffs[i] = 0;
        for (int j = 0; j < size; j++) {
            diffs[i] += prod[j][i];
        }
    }


    // TODO fix recursive structure...
    _free_array2d((void**)cross, size);
    _free_array2d((void**)wt_transpose, size);
    _free_array2d((void**)prod, size);

    for (int i = 0; i < size; i++) {
        if (diffs[i] < 0) {
            spins[i] *= -1;

            return cobi_simple_descent(spins, weights);
        }
    }
    // return ham;
}

// ising subproblem solver

CobiData *cobi_data_mk(size_t size, int chip_delay, bool descend)
{
    CobiData *d = (CobiData*)malloc(sizeof(CobiData));
    d->probSize = size;
    d->w = 64;
    d->h = 64;
    d->programming_bits = _malloc_array2d(d->w, d->h);
    d->chip2_test = (uint8_t*)malloc(sizeof(uint8_t) * 441);

    d->num_samples = 1;
    d->spins = _malloc_array1d(NUM_GROUPS);

    d->chip_delay = chip_delay;

    d->descend = descend;

    return d;
}

void free_cobi_data(CobiData *d)
{
    _free_array2d((void**)d->programming_bits, d->w);
    free(d->chip2_test);
    free(d->spins);
    free(d);
}

int cobi_gpio_setup()
{
        gpioSetMode(WEIGHT_2,    PI_OUTPUT);
        gpioSetMode(WEIGHT_3,    PI_OUTPUT);
        gpioSetMode(SCANOUT_CLK, PI_OUTPUT);
        gpioSetMode(SAMPLE_CLK,  PI_OUTPUT);
        gpioSetMode(ALL_ROW_HI,  PI_OUTPUT);
        gpioSetMode(WEIGHT_1,    PI_OUTPUT);
        gpioSetMode(WEIGHT_EN,   PI_OUTPUT);
        gpioSetMode(COL_ADDR_4,  PI_OUTPUT);
        gpioSetMode(ADDR_EN64_CHIP1, PI_OUTPUT);
        gpioSetMode(COL_ADDR_3,  PI_OUTPUT);
        gpioSetMode(COL_ADDR_1,  PI_OUTPUT);
        gpioSetMode(COL_ADDR_0,  PI_OUTPUT);
        gpioSetMode(ROW_ADDR_2,  PI_OUTPUT);
        gpioSetMode(ROW_ADDR_3,  PI_OUTPUT);
        gpioSetMode(WEIGHT_5,    PI_OUTPUT);
        gpioSetMode(SCANOUT_DOUT64_CHIP2, PI_INPUT);
        gpioSetMode(SCANOUT_DOUT64_CHIP1, PI_INPUT);
        gpioSetMode(WEIGHT_0,    PI_OUTPUT);
        gpioSetMode(ROSC_EN,     PI_OUTPUT);
        gpioSetMode(COL_ADDR_5,  PI_OUTPUT);
        gpioSetMode(ROW_ADDR_5,  PI_OUTPUT);
        gpioSetMode(ADDR_EN64_CHIP2, PI_OUTPUT);
        gpioSetMode(WEIGHT_4,    PI_OUTPUT);
        gpioSetMode(COL_ADDR_2,  PI_OUTPUT);
        gpioSetMode(ROW_ADDR_1,  PI_OUTPUT);
        gpioSetMode(ROW_ADDR_0,  PI_OUTPUT);
        gpioSetMode(ROW_ADDR_4,  PI_OUTPUT);

        GPIO_WRITE(SAMPLE_CLK, PI_LOW);

        return 0;
}

void cobi_weight_pins_low()
{
        GPIO_WRITE(WEIGHT_0, PI_LOW);
        GPIO_WRITE(WEIGHT_1, PI_LOW);
        GPIO_WRITE(WEIGHT_2, PI_LOW);
        GPIO_WRITE(WEIGHT_3, PI_LOW);
        GPIO_WRITE(WEIGHT_4, PI_LOW);
        GPIO_WRITE(WEIGHT_5, PI_LOW);
}

void cobi_set_addr(const int *addrs, int *bin_num_list)
{
    int addr_name;
    int i;
    for (i = 0; i < 6; i++) {
        addr_name = addrs[i];
        if (bin_num_list[i] == 1) {
            GPIO_WRITE(addr_name, PI_HIGH);
        } else {
            GPIO_WRITE(addr_name, PI_LOW);
        }
    }
}

void cobi_program_weights(int **programming_bits)
{
    if (Verbose_ > 0) {
        printf("Programming chip\n");
    }

    int enable_pin_name = ADDR_EN64_CHIP2;

    // initialize binary lists
    int bin_row_list[6];
    int bin_col_list[6];
    int bin_weight_list[6];

    // reset pins for programming
    cobi_weight_pins_low();
    GPIO_WRITE(ALL_ROW_HI, PI_LOW);

    // # run through each row of 64x64 cells in COBI/COBIFREEZE
    int x = 0;
    int y = 0;
    for (x = 0; x < 64; x++) { // #run through each row of 64x64 cells in COBI/COBIFREEZE
        binary_splice_rev(x, bin_row_list);
        for (y = 0; y < 64; y++) { // #run through each cell in a given row
            binary_splice_rev(y, bin_col_list);
            binary_splice_rev(programming_bits[x][y], bin_weight_list);

            cobi_set_addr(ROW_ADDRS, bin_row_list); // #assign the row number
            cobi_set_addr(COL_ADDRS, bin_col_list); // #assign the column number

            GPIO_WRITE(enable_pin_name, PI_HIGH);

            // #set weight of 1 cell
            cobi_set_addr(WEIGHTS, bin_weight_list); // #assign the weight corresponding to current cell
            // # time.sleep(.001) # Delay removed since COBIFIXED65 board does not have any level shifters which causes additional signal delay
            GPIO_WRITE(enable_pin_name, PI_LOW);
            cobi_weight_pins_low(); // #reset for next address
        }
    }

    if (Verbose_ > 0) {
        printf("Programming completed\n");
    }
}

/*
 * cobi_read_spins
 *
 * returns spins via `cobi_data->spins`
 */
void cobi_read_spins(CobiData *cobi_data)
{
    // chip_data_len must equal 63*7 == 441
    // int const chip_data_len = 441;

    int excess_0s[63];

    int node_index = 0;
    int cur_val = 0;

    // reverse order of nodes
    for (int i = 0; i < 63; i++) {
        excess_0s[i] = 0;
        node_index = (62 - i) * 7;

        for (int bit_index = 0; bit_index < 7; bit_index++) {
            cur_val = cobi_data->chip2_test[node_index+bit_index];

            if (cur_val == 0) {
                excess_0s[i]++;
            } else {
                excess_0s[i]--;
            }
        }
    }

    for (int g = 0; g < NUM_GROUPS; g++) {
        node_index = COBIFIXED65_BASEGROUPS[g];
        if (excess_0s[node_index] <= 0) {
            cobi_data->spins[g] = -1;
        } else {
            cobi_data->spins[g] = 1;
        }
    }
}

void cobi_gh_cal_energy_direct(int *spins, int **weights, int *hamiltonian, bool descend)
{
    int size = NUM_GROUPS;

    // implementing only the `descend == True` path in original code
    if (descend){
        cobi_simple_descent(spins, weights);
    }

    int ham = 0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            ham = ham + spins[i] * spins[j] * weights[i][j];
        }
    }
    *hamiltonian = ham;
    if (Verbose_ > 2) {
        printf("Hamiltonian: %d\n", ham);
    }
}

void cobi_gh_cal_energy(CobiData *cobi_data, int *hamiltonian)
{
    cobi_read_spins(cobi_data);

    /* weights = np.zeros((num_groups,num_groups),dtype=np.int8) */
    int **weights = _malloc_array2d(NUM_GROUPS, NUM_GROUPS);

    /* graph_arr = np.zeros((64,64),dtype=np.int8) */
    /* graph_arr = import_graph(graph_arr,graph_file) */
    /* int **graph_arr = all_to_all_graph_write_0; */

    int x, y, i, j;
    for (x = 0; x < NUM_GROUPS; x++) {
        for (y = x + 1; y < NUM_GROUPS; y++){
            /* for i in groups[x]: */
            /*     for j in groups[y]: */
            i = COBIFIXED65_BASEGROUPS[x];
            j = COBIFIXED65_BASEGROUPS[y];

            weights[x][y] -= (cobi_data->programming_bits[62-i][j+1] +
                              cobi_data->programming_bits[62-j][i+1]);
        }
    }

    if (Verbose_ > 1) {
        printf("Spins before: ");
        for (i = 0; i < NUM_GROUPS; i++) {
            printf(" %d", cobi_data->spins[i]);
        }
        printf("\n");
    }

    /* return cal_energy_direct(spins,weights,descend,return_spins) */
    cobi_gh_cal_energy_direct(cobi_data->spins, weights, hamiltonian, cobi_data->descend);

    if (Verbose_ > 1) {
        printf("Spins after: ");
        for (i = 0; i < NUM_GROUPS; i++) {
            printf(" %d", cobi_data->spins[i]);
        }
        printf("\n");
    }

    _free_array2d((void**)weights, NUM_GROUPS);
}

// cobi_data_array is not used
int cobi_cal_energy(CobiData *cobi_data)
{
    /* #if sample_index%3==0: */
    GPIO_WRITE(ROSC_EN, PI_LOW);
    GPIO_WRITE_DELAY(ROSC_EN, PI_HIGH, cobi_data->chip_delay);

    // usleep(cobi_data->chip_delay);

    GPIO_WRITE_DELAY(SAMPLE_CLK, PI_HIGH, cobi_data->chip_delay);
        /* #time.sleep(0.0001) */
    // usleep(cobi_data->chip_delay);
    GPIO_WRITE_DELAY(SAMPLE_CLK, PI_LOW, cobi_data->chip_delay);

    int bit = 0;
    for (bit = 0; bit < 441; bit++) {
        /*     # if (sample == 1) and (bit < 64): */
        /*     #     print(GPIO.input(scanout_dout64_chip1)) */
        if (gpioRead(SCANOUT_DOUT64_CHIP2) == 1) {
            cobi_data->chip2_test[bit] = 1;

        } else {
            cobi_data->chip2_test[bit] = 0;
        }

        if (bit == 440) {
            break;
        }

        GPIO_WRITE(SCANOUT_CLK, PI_HIGH);
        GPIO_WRITE(SCANOUT_CLK, PI_LOW);
    }

    /* int *hamiltonians = malloc(sizeof(double) * num_samples); */
    int hamiltonian = 0;
    cobi_gh_cal_energy(cobi_data, &hamiltonian);

    // TODO add majority voting thing..
    return hamiltonian;
}

void cobi_modify_array_for_pins(int **initial_array, int  **final_pin_array, int problem_size)
{
    int total_0_rows = 63 - problem_size;

    int y_diag = problem_size; //#set to y-location at upper right corner of problem region

    // #part 1: adjust all values within problem regions of array
    int x, y, integer_pin;
    for (x = total_0_rows; x < 63; x++) {
        for (y = 1; y < problem_size + 1; y++) {
            integer_pin = initial_array[x][y];

            if (integer_pin == -7) {
                final_pin_array[x][y] = 0b001110; // # load value of 14.0 to final_array
            } else if (integer_pin == -6) {
                final_pin_array[x][y] = 0b001100;     // # load value of 12.0 to final_array

            } else if (integer_pin == -5) {
                final_pin_array[x][y] = 0b001010;     // # load value of 10.0 to final_array

            } else if (integer_pin == -4) {
                final_pin_array[x][y] = 0b001000;     // # load value of 8.0 to final_array

            } else if (integer_pin == -3) {
                final_pin_array[x][y] = 0b000110;     // # load value of 6.0 to final_array

            } else if (integer_pin == -2) {
                final_pin_array[x][y] = 0b000100;     // # load value of 4.0 to final_array

            } else if (integer_pin == -1) {
                final_pin_array[x][y] = 0b000010;     // # load value of 2.0 to final_array

            } else if (integer_pin == 0) {
                final_pin_array[x][y] = 0b000000;     // # load value of 0.0 to final_array

            } else if (integer_pin == 1) {
                final_pin_array[x][y] = 0b000011;     // # load value of 3.0 to final_array

            } else if (integer_pin == 2) {
                final_pin_array[x][y] = 0b000101;     // # load value of 5.0 to final_array

            } else if (integer_pin == 3) {
                final_pin_array[x][y] = 0b000111;     // # load value of 7.0 to final_array

            } else if (integer_pin == 4) {
                final_pin_array[x][y] = 0b001001;     // # load value of 9.0 to final_array

            } else if (integer_pin == 5) {
                final_pin_array[x][y] = 0b001011;     // # load value of 11.0 to final_array

            } else if (integer_pin == 6) {
                final_pin_array[x][y] = 0b001101;     // # load value of 13.0 to final_array

            } else if (integer_pin == 8) {
                final_pin_array[x][y] = 0b011111;     // # load the strong positive coupling to the final_array
            } else if (integer_pin == -8) {
                final_pin_array[x][y] = 0b111110;     // # load the strong negative coupling to the final_array
            } else { // # integer_pin == 7
                // TODO rectify value in comment with actual value being assigned
                if (y == y_diag) { // #along diagonal
                    final_pin_array[x][y] = 0b001111; // # load value of 31.0 to final_array
                } else {
                    final_pin_array[x][y] = 0b001111; // # load value of 15.0 to final_array

                }
            }
        }
        y_diag--;
    }

    // #part 2 - adjust remaining 7s in diagonal
    y_diag = problem_size;
    for (x = 0; x < 64; x++) {
        for (y = 0; y < 64; y++) {
            integer_pin = initial_array[x][y];
            if (y == y_diag && integer_pin == 7) {
                final_pin_array[x][y] = 0b001111;
            }
        }
    }
    /* return final_pin_array; */
}

// py: cobifixed65_rpi::test_multi_times
int *cobi_test_multi_times(
    CobiData *cobi_data, int sample_times, int size, int8_t *solution
) {
    cobi_program_weights(cobi_data->programming_bits);

    int times = 0;
    int *all_results = _malloc_array1d(sample_times);
    int cur_best = 0;
    int res;

    /* int energy_ham = cobi_cal_energy_ham(...); //# calculate Qbsolv energy once */
    // int energy_ham = 0;
    /* int **cobi_data_array = _malloc_array2d(400,sample_times); */

    GPIO_WRITE(ALL_ROW_HI, PI_HIGH);
    GPIO_WRITE(SCANOUT_CLK, PI_LOW);
    GPIO_WRITE(WEIGHT_EN, PI_HIGH);
    GPIO_WRITE_DELAY(WEIGHT_EN, PI_LOW, cobi_data->chip_delay);

    while (times < sample_times) {
        if (Verbose_ > 2) {
            printf("\nSample number %d\n", times);
        }
        res = cobi_cal_energy(cobi_data);  //# calculate H energy from chip data

        all_results[times] = res;

        if (res > cur_best) {
            cur_best = res;
            for (int i = 0; i < NUM_GROUPS; i++ ) {
                solution[i] = cobi_data->spins[i];
            }
        }

        times += 1;

        if (Verbose_ > 2) {
            printf(", best = %d ",  cur_best);
        }
    }

    GPIO_WRITE(ALL_ROW_HI, PI_LOW);

    if (Verbose_ > 2) {
        printf("Finished!\n");
    }

    return all_results;
}

int **cobi_init_problem_matrix(int **problem_data, int problem_size)
{
    if (problem_size > NUM_GROUPS) {
        printf("Bad problem size: %d\n", problem_size);
        exit(1);
    }

    int **m = _malloc_array2d(64, 64);

    int i, j;
    for (i = 0; i < 64; i++) {
        for (j = 0; j < 64; j++) {
            m[i][j] = BLANK_GRAPH[i][j];
        }
    }

    for (int x = 0; x < problem_size; x++) {
        i = COBIFIXED65_BASEGROUPS[x];
        for (int y = 0; y < problem_size; y++) {

            j = COBIFIXED65_BASEGROUPS[y];

            m[62-i][j+1] = problem_data[x][y];
        }
        m[62-i][i+1] = 7;
    }

    return m;
}

// Normalize
void cobi_norm_val(int **norm, double **ising, size_t size)
{
    // TODO consider alternate mapping/normalization schemes
    double min = 0;
    double max = 0;
    double cur_v = 0;

    size_t i,j;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            cur_v = ising[i][j];

            if (cur_v > max) max = cur_v;
            if (cur_v < min) min = cur_v;
        }
    }

    // Linear scaling to range [-14, 14]
    // (y + 14) / (x - min) = 28 / (max - min)
    // y = (28 / (max - min)) * (x - min) - 14

    for (i = 0; i < size; i++) {
        for (j = i; j < size; j++) {
            cur_v = ising[i][j];
            if (cur_v == 0) {
                norm[i][j] = 0;
                norm[j][i] = 0;
            } else if (i == j) {
                double scaled = (28 * (cur_v - min)/ (max - min)) - 14;
                  norm[i][i] = scaled;
            } else {
                double scaled = ((28 * (cur_v - min)) / (max - min)) - 14;
                int symmetric_val = (int) round(scaled / 2);
                norm[i][j] = symmetric_val;
                norm[j][i] = symmetric_val;
            }
        }
    }
}

void ising_solution_from_qubo_solution(int8_t *ising_soln, int8_t *qubo_soln, int len)
{
    // Convert solution to ising formulation
    for(int i = 0; i < len; i++) {
        if (qubo_soln[i] == 1) {
            ising_soln[i] = 1;
        } else {
            ising_soln[i] = -1;
        }
    }
}

void qubo_solution_from_ising_solution(int8_t *qubo_soln, int8_t *ising_soln, int len)
{
    for(int i = 0; i < len; i++) {
        if (ising_soln[i] == 1) {
            qubo_soln[i] = 1;
        } else {
            qubo_soln[i] = 0;
        }
    }
}


// Converts problem from qubo to ising formulation.
// Result is stored in `ising`.
void ising_from_qubo(double **ising, double **qubo, int size)
{
    for (int i = 0; i < size; i++) {
        ising[i][i] = qubo[i][i] / 2;

        for (int j = i + 1; j < size; j++) {
            // convert quadratic terms
            ising[i][j] = qubo[i][j] / 4;
            ising[j][i] = qubo[j][i] / 4;

            // add rest of linear term
            ising[i][i] += ising[i][j] + ising[j][i];
        }
    }
}

int cobi_init()
{
    if (gpioInitialise() < 0) return 1;

    // setup GPIO pins
    cobi_gpio_setup();

    if(Verbose_ > 0) printf("GPIO initialized successfully\n");

    return 0;
}

bool cobi_established()
{
    // TODO How to verify existence of cobi chip?
    // connection = getenv("DW_INTERNAL__CONNECTION");
    // if (connection == NULL) {
    return true;
    // }
    // return true;
}

void cobi_solver(
    double **qubo, int numSpins, int8_t *qubo_solution, int num_samples, int chip_delay, bool descend
) {
    if (numSpins > 59) {
        printf("Quitting.. cobi_solver called with size %d. Cannot be greater than 59.\n", numSpins);
        exit(2);
    }

    CobiData *cobi_data = cobi_data_mk(numSpins, chip_delay, descend);
    int8_t *ising_solution = (int8_t*)malloc(sizeof(int8_t) * numSpins);
    double **ising = _malloc_double_array2d(numSpins, numSpins);
    int **norm_ising = _malloc_array2d(numSpins, numSpins);

    ising_from_qubo(ising, qubo, numSpins);
    cobi_norm_val(norm_ising, ising, numSpins);

    int **mtx = cobi_init_problem_matrix(norm_ising, numSpins);
    cobi_modify_array_for_pins(mtx, cobi_data->programming_bits, 63);

    // Convert solution from QUBO to ising
    ising_solution_from_qubo_solution(ising_solution, qubo_solution, numSpins);

    //
    int *results = cobi_test_multi_times(
        cobi_data, num_samples, numSpins, ising_solution
    );

    // Convert ising solution back to QUBO form
    qubo_solution_from_ising_solution(qubo_solution, ising_solution, numSpins);

    free(results);
    free_cobi_data(cobi_data);
    free(ising_solution);
    _free_array2d((void**)ising, numSpins);
    _free_array2d((void**)norm_ising, numSpins);
    _free_array2d((void**)mtx, 64);
}

void cobi_close()
{
    if (Verbose_ > 0) {
        printf("pigpio clean up\n");
    }

    gpioTerminate();
}

#ifdef __cplusplus
}
#endif
