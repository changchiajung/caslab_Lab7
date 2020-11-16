/* Necessary Headers Include */
#include "embARC.h"
#include "embARC_debug.h"

/* Define Max Number for 32-bit Integer */
#define MAX_COUNT 0xffffffff

/**
 * 宣告或定義你喜歡的埠(port)與腳位(pin)供超音波測距模組使用
 * e.g.:
 * #define UltraSonicPort DFSS_GPIO_8B2_ID
 * {
 */
#define UltraSonicPort DFSS_GPIO_4B2_ID
#define UltraSonicPin 0
/**
 * }
 */

/**
 * 宣告 GPIO 物件
 * {
 */
DEV_GPIO_PTR pUltraSonicGpioObject;
/**
 * }
 */

/**
 * 宣告用來判斷是否有被中斷的變數
 * {
 */
volatile bool isInterrupted = false;
/**
 * }
 */

/* 宣告Timer0_ISR的原型 */
void Timer0_ISR();

int main(void) {
	/**
	 * 獲得signal腳位的GPIO物件
	 * 將signal腳位設為輸出腳位
	 * {
     */
	pUltraSonicGpioObject = gpio_get_dev(UltraSonicPort);
	pUltraSonicGpioObject->gpio_open((1 << UltraSonicPin));	
	
	/**
	 * }
	 */

	while (1) {
		/**
		 * 將超音波signal腳位設為輸出
		 * {
		 */
		pUltraSonicGpioObject->gpio_control(GPIO_CMD_SET_BIT_DIR_OUTPUT, (void *)(1 << UltraSonicPin));
		/**
		 * }
		 */

		/**
		 * 將TIMER_0停止
		 * 裝載(install) Timer0_ISR為TIMER0中斷的處理程序(handler)
		 * 開啟TIMER_0的interrupt
		 * 啟動TIMER_0，設定為啟動中斷(Interrupt Enabled)模式，並且在5us(5*10^-6s)時中斷
		 * {
		 */
		timer_stop(TIMER_0);
		int_handler_install(INTNO_TIMER0, Timer0_ISR);
		timer_enable(INTNO_TIMER0);
		timer_start(TIMER_0, TIMER_CTRL_IE, 0.000005 * CLK_CPU);
		/**
		 * }
		 */

		/**
		 * 將超音波signal腳位設為高電位
		 * {
		 */
                pUltraSonicGpioObject->gpio_write(1 << UltraSonicPin, 1 << UltraSonicPin);
		
		/**
		 * }
		 */

		/**
		 * 設定用來判斷是否有中斷的變數
		 * 等待該變數在中斷改變
		 * 將signal pin腳設定為低電位
		 * {
		*/
		isInterrupted = false;
                while (!isInterrupted) ;		
		pUltraSonicGpioObject->gpio_write(0 << UltraSonicPin, 1 << UltraSonicPin);
		/**
		 * }
		 */

		/**
		 * 停止TIMER_0
		 * 將signal腳位設定為輸入腳位
		 * 宣告一個變數來儲存signal腳位的電位值
		 * 等待signal腳位變為高電位
		 * {
		 */
		 timer_stop(TIMER_0);
                 pUltraSonicGpioObject->gpio_control(GPIO_CMD_SET_BIT_DIR_INPUT, (void *)(1 << UltraSonicPin));
                 uint32_t UltraSonicPinValue = 0;
                 while (UltraSonicPinValue == 0) { pUltraSonicGpioObject->gpio_read(&UltraSonicPinValue, 1 << UltraSonicPin); }
		/**
		 * }
		 */

		/**
		 * 利用timer_start開始計數，因為不需要interrupt所以可以將mode設為0或是TIMER_CTRL_NH，並且設定val為最大值
		 * 待signal腳位變為低電位
		 * {
		 */
		timer_start(TIMER_0, 0, MAX_COUNT);
		while (UltraSonicPinValue != 0) { pUltraSonicGpioObject->gpio_read(&UltraSonicPinValue, 1 << UltraSonicPin); }

		
		/**
		 * }
		 */

		/**
		 * 利用一個變數紀錄TIMER_0目前的計數，作為開始到結束經過的CPU cycle數
		 * 透過開始到結束經過的CPU Cycle數量除以CPU Cycle(CLK_CPU)得到秒數
		 * 再乘以 340(m/s)*100(cm/m)/2(來回) 得到公分數
		 * 透過序列埠印出透過超音波測距得到的公分數
		 * {
		 */
		 uint32_t DurationCnt;
                 timer_current(TIMER_0, &DurationCnt);
                 float Distance = ((float)DurationCnt * 17000) / CLK_CPU;
                 EMBARC_PRINTF("Distance = %f\n", Distance);		
		/**
		 * }
		 */
	}
	return E_SYS;
}

void Timer0_ISR() {
	/**
	 * 先將TIMER_0的 interrupt 清除
	 * 設定該設定的變數
	 * {
	 */
	timer_int_clear(TIMER_0);
	isInterrupted = true;	
	/**
	 * }
	 */
}
