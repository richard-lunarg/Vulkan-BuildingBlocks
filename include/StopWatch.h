/* Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Copyright © 2023 Richard S. Wright Jr. (richard@lunarg.com)
 *
 * This software is part of the Vulkan Building Blocks
 */

#pragma once
#include <chrono>

// ***********************************************************************
// Simple Stopwatch class. Use this for high resolution timing
// purposes (or, even low resolution timings)
// Pretty self-explanitory.... 
// Reset(), or GetElapsedSeconds().
class StopWatch
	{
	public:
		StopWatch(void)	// Constructor
			{
            start = std::chrono::high_resolution_clock::now();
			}

		// Resets timer (difference) to zero
		inline void reset(void) 
			{
            start = std::chrono::high_resolution_clock::now();
			}
		
		// Get elapsed time in seconds
		double getElapsedSeconds(void)
			{
            end = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> diff = end - start;
            return diff.count();
            }
	
	protected:
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
        std::chrono::time_point<std::chrono::high_resolution_clock> end;
	};



