/*
   Tree v2: https://github.com/evilgeniuslabs/tree-v2
   Copyright (C) 2016 Jason Coon

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

const uint8_t radii[NUM_LEDS] = {
  57, 142, 150, 150, 142, 57, 57, 142, 150, 150, 142, 57, 57, 142, 150, 150, 142, 57, 57, 142, 150, 150, 142, 57, 57, 142, 150, 150, 142, 57, 57, 113, 150, 150, 113, 57, 57, 113, 150, 150, 113, 57, 57, 113, 150, 150, 113, 57, 57, 113, 150, 150, 113, 57, 57, 113, 150, 150, 113, 57, 57, 113, 150, 150, 113, 57, 47, 123, 123, 47, 47, 123, 123, 47, 47, 123, 123, 47, 47, 123, 123, 47, 47, 123, 123, 47, 47, 123, 123, 47, 113, 113, 113, 113, 113, 113, 113, 113, 94, 94, 94, 94, 94, 94, 94, 76, 76, 76, 76, 76, 76, 76, 57, 57, 57, 57, 57, 57, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 0, 0, 0, 0
};

const uint8_t conicalRadii[NUM_LEDS] = {
  0, 1, 2, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 1, 2, 2, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

const uint8_t angles[NUM_LEDS] = {
  0, 0, 0, 0, 43, 43, 43, 43, 85, 85, 85, 85, 128, 128, 128, 128, 21, 21, 21, 21, 64, 64, 64, 64, 107, 107, 107, 107, 149, 149, 149, 149, 0, 0, 0, 0, 43, 43, 43, 43, 85, 85, 85, 85, 128, 128, 128, 128, 21, 21, 21, 21, 64, 64, 64, 64, 107, 107, 107, 107, 149, 149, 149, 149, 0, 0, 0, 0, 43, 43, 43, 43, 85, 85, 85, 85, 128, 128, 128, 128, 149, 149, 21, 21, 64, 64, 107, 107, 0, 32, 64, 96, 128, 18, 55, 91, 128, 18, 55, 91, 128, 0, 43, 85, 128, 0, 64, 96, 32, 43, 128, 213, 0, 85, 64, 64
};

const uint8_t levelCount = 16;
const uint8_t ledsPerLevel[levelCount] = { 36, 36, 36, 36, 36, 24, 8, 7, 7, 6, 4, 4, 3, 3, 2, 2};

const uint8_t levelStart[levelCount] = {
  0,
  3,
  7,
  10,
  44,
  80,
  104,
  112,
  119,
  126,
  132,
  136,
  140,
  143,
  146,
  148
};

const uint8_t levelEnd[levelCount] = {
  3,
  7,
  7,
  43,
  79,
  103,
  111,
  118,
  125,
  131,
  135,
  139,
  142,
  145,
  147,
  149
};

const uint8_t levels[NUM_LEDS] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
  6, 6, 6, 6, 6, 6, 6, 6,
  7, 7, 7, 7, 7, 7, 7,
  8, 8, 8, 8, 8, 8, 8,
  9, 9, 9, 9, 9, 9,
  10, 10, 10, 10,
  11, 11, 11, 11,
  12, 12, 12,
  13, 13, 13,
  14, 15,
  15, 14
};

const uint8_t zCoords[NUM_LEDS] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
    52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
    82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
    112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112,
    138, 138, 138, 138, 138, 138, 138, 138, 138, 138, 138, 138, 138, 138, 138, 138, 138,
    150, 150, 150, 150, 150, 150, 150, 150
};

const uint8_t xCoords[NUM_LEDS] = {
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 91, 54, 12, 12, 54, 91, 91, 54, 12, 12, 54, 91, 109, 88, 65, 65, 88, 109, 146, 146, 109, 88, 65, 65, 88, 109, 90, 47, 0, 0, 47, 90, 109, 88, 65, 65, 88, 109, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 95, 58, 17, 17, 58, 95, 94, 52, 9, 9, 52, 94, 111, 90, 69, 69, 90, 111, 144, 144, 111, 90, 69, 69, 90, 111, 99, 74, 49, 49, 74, 99, 99, 74, 49, 49, 74, 99, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 114, 95, 95, 114, 99, 61, 61, 99, 114, 95, 95, 114, 141, 141, 84, 66, 84, 128, 128, 87, 77, 105, 150, 128, 94, 86, 109, 146, 128, 99, 99, 128, 150, 150, 128, 99, 128, 150, 148, 107, 107
};

const uint8_t yCoords[NUM_LEDS] = {
  148, 148, 107, 87, 65, 65, 87, 107, 87, 46, 0, 0, 46, 87, 107, 87, 65, 65, 87, 107, 148, 148, 128, 128, 128, 128, 96, 60, 21, 21, 60, 96, 96, 60, 21, 21, 60, 96, 128, 128, 128, 128, 128, 128, 150, 150, 145, 145, 110, 89, 67, 67, 89, 110, 91, 50, 5, 5, 50, 91, 110, 89, 67, 67, 89, 110, 128, 128, 128, 150, 150, 128, 128, 128, 128, 100, 64, 28, 28, 64, 100, 100, 64, 28, 28, 64, 100, 112, 98, 85, 85, 98, 112, 143, 150, 150, 143, 150, 150, 143, 112, 98, 85, 85, 98, 112, 96, 68, 41, 41, 68, 96, 104, 72, 72, 104, 128, 128, 128, 128, 150, 150, 150, 150, 128, 128, 128, 128, 104, 72, 72, 104, 86, 128, 128, 86, 68, 77, 97, 138, 138, 97, 87, 102, 136, 136, 102, 96
};
