#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"

#define NUMBER 1
#define LETTER 0

#define A LL_GPIO_PIN_0
#define B LL_GPIO_PIN_1
#define C LL_GPIO_PIN_2
#define D LL_GPIO_PIN_3
#define E LL_GPIO_PIN_4
#define F LL_GPIO_PIN_5
#define G LL_GPIO_PIN_6


static const uint32_t decoderN[] = {
	0,// " "
	A | B | C | D | E | F, // 0
	B | C, // 1
	A | B | G | E | D, // 2
	A | B | G | C | D, // 3
	F | G | B | C, //4
	A | C | D | F | G, // 5
	A | C | D | E | F | G, // 6
	A | B | C, // 7
	A | B | C | D | E | F | G, //8
	A | B | C | D | F | G, // 9
};

static const uint32_t decoderL[] = {
    0,// " "
	A | B | C | G | F | E, // A
	C | D | E | F | G, // b
	A | D | E | F, // C
	C | D | E | B | G, // d
	A | D | E | F | G, // E
	A | E | F | G,// F
	B | C | E | F | G, // H
	F | E | D,//L
	F | E | A | B | G //P
};

static void rcc_config()
{
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1);
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2,
                                LL_RCC_PLL_MUL_12);
    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1);
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    SystemCoreClock = 48000000;
}

static void gpio_config(void)
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);//включили тактирование порта
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);//активировать режим цифровой вход
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_4, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_1, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_2, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_3, LL_GPIO_MODE_OUTPUT);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_0, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_1, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_2, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_3, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_4, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_5, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_6, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);
    return;
}

void show_digit(int digit)
{
    uint32_t out = decoderN[digit];
    LL_GPIO_WriteOutputPort(GPIOB, out); //то что приготовили в аут пишем в порт б
}

void switcher(uint32_t port_state, int digit_num, uint32_t position, int f)
{
	static uint32_t mask = A | B | C | D| E | F | G;

	port_state = (port_state & ~mask) | ((f)? decoderN[position] : decoderL[position]);
    LL_GPIO_WriteOutputPort(GPIOB, port_state);

    port_state = LL_GPIO_ReadOutputPort(GPIOC);
    

    port_state = (port_state & ~mask) | ~(1 << (4 - digit_num));
    LL_GPIO_WriteOutputPort(GPIOC, port_state);

}


void run_string(uint32_t* stringLetter, short n)
{
	static int i = 0;
	static counter = 0;
	int number = stringLetter[i % n]*1000 + stringLetter[(i+1) % n]*100 + stringLetter[(i+2) % n]*10 + stringLetter[(i+3) % n];
	counter++;
	if (counter % 100 == 0) // сколько раз слово будет показываться на одном месте
		i++;
	static int digit_num = 0;

	uint32_t port_state = 0;
    port_state = LL_GPIO_ReadOutputPort(GPIOB);
    uint32_t position;
    if (digit_num == 0)
        position = number % 10;
    if (digit_num == 1)
        position = (number / 10) % 10;
    if (digit_num == 2)
        position = (number / 100) % 10;
    if (digit_num == 3)
        position = (number / 1000) % 10;
    switcher(port_state, digit_num, position, LETTER);
    digit_num = (digit_num + 1) % 4;
}

__attribute__((naked)) static void delay(void)
{
    asm ("push {r7, lr}");
    asm ("ldr r6, [pc, #8]");
    asm ("sub r6, #1");
    asm ("cmp r6, #0");
    asm ("bne delay+0x4");
    asm ("pop {r7, pc}");

    asm (".word 0x5b8d8"); //6000000
}


__attribute__((naked)) static void delay_10ms(void)
{
    asm ("push {r7, lr}");
    asm ("ldr r6, [pc, #8]");
    asm ("sub r6, #1");
    asm ("cmp r6, #0");
    asm ("bne delay_10ms+0x4");
    asm ("pop {r7, pc}");
    asm (".word 0x3a60"); //60000
}

void run();

int main(void)
{
    rcc_config();
    gpio_config();

    run();

    return 0;
}

void run()
{
	int a[] = {7, 5, 8, 9, 0, 0, 0, 0};//help
    int k;
    int status = 0;
	int oldstatus = 0;
	while (1)
	{
		run_string(a, 8);
		k = 3;
		while (k--)
			delay_10ms(); // время переключения между буквами в одном цикле
	}
}

void display()
{
	int status = 0;
	int oldstatus = 0;
	int k = 0;

	while(1)
	{
		int number = 0;

		status = LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0);
		if (status && !oldstatus) {
	            dyn_display(number);
	            number++;
	    }
		delay();
		
	}
}