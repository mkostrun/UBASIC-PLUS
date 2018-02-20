/**
 ******************************************************************************
 * File Name          : util_haltick.c
 * Description        : HAL based ticker utility functions
 ******************************************************************************
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "cube_hal.h"

/**
 * @brief  Finds time passed between instances t1 (earlier) and t2 (later)
 * @param  t1 earlier time
 * @param  t2 later time
 * @retval dt duration of time between two time instances
 */
uint32_t delta_t(uint32_t t1, uint32_t t2 )
{
  if (t2==t1)
    return 0;

  uint32_t rval=t2;

  if (t2 > t1)
    rval -= t1;
  else
  {
    uint32_t m1=0xffffffff-t1;
    rval += m1;
  }

  return rval;
}

/**
 * @brief  update t1 with t2 only if t2 - t1 >= t_limit
 * @param  t1 ptr to earlier time
 * @param  t2 later time
 * @retval dt duration of time between two time instances
 */
uint8_t update_after_delta_t(uint32_t *t1, uint32_t t2, uint32_t t_limit)
{
  if ((*t1)==t2)
    return 0;

  uint32_t rval=t2;
  if (t2 > *t1)
    rval -= (*t1);
  else
  {
    uint32_t m1=0xffffffff-(*t1);
    rval += m1;
  }

  if (rval < t_limit)
    return (0);

  *t1 = t2;
  return (1);
}
