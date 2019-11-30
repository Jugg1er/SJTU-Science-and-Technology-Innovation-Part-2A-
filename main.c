/*
按键1：选择MODE	   基础0  音乐1 波形2  默认为0
按键2：选择哪种波形或哪首音乐
按键3：调整振幅-
按键4：调整振幅+
按键5：调整频率-
按键6：调整频率+
按键8：是否启用增益反馈	0：不启用 1：启用 默认为0
P1.0~P1.3 基础部分
P1.4 增益反馈
P1.5~P2.0 波形	输入到单片机的电压不能超过2.5V  //因为ADC参考电压为2.5伏，满量程即为2.5
P2.2 音乐
P2.4 红外遥控
*/

//T_A0*100 = A0周期
unsigned char T_A0 = 50;//  用于音乐播放和函数信号发生中的频率调整

//红外脉冲时间(ms)			short1*10/T_A0
unsigned short1 = 60;
unsigned short2 = 100;
unsigned long1 = 140;
unsigned long2 = 180;   //通过示波器观察，短脉冲为80ms，长脉冲为160ms，因此取正负20ms为区间

unsigned char input2_4;
unsigned input_adder = 0;
unsigned input_status = 0;		//0:no	1;short	  2:long

int sample;
double volt;//AD转换后的输入值与实际值
double Vmax = 1.5;
double Vmin = 0.4;

unsigned char number = 255;

// 8位数码管显示的数字或字母符号
// 注：板上数码位从左到右序号排列为4、5、6、7、0、1、2、3
// 主程序中digi[]按45670123写
// 从外面看来：从左到右：第1位控制模式，第2位显示当前播放第几首音乐或产生第几种波形，4显示是否自动增益
// 第5,6位显示控制频率，第7,8位控制振幅，
unsigned char digit[8]={'-','-','-','-','-','-','-','-'};

// 8位小数点 1亮  0灭
// 注：板上数码位小数点从左到右序号排列为4、5、6、7、0、1、2、3
unsigned char pnt=0x04;

// 8个LED指示灯状态，每个灯4种颜色状态，0灭，1绿，2红，3橙（红+绿）
// 注：板上指示灯从左到右序号排列为7、6、5、4、3、2、1、0
//     对应元件LED8、LED7、LED6、LED5、LED4、LED3、LED2、LED1
unsigned char led[]={0,0,0,0,0,0,0,0};
unsigned char gain_state = 10;//开机启动默认增益
// 当前按键值
unsigned char key_state = 0, key_flag = 1, key_code = 0;

#define IR1 5
//40 8
#define IR2 15
//100 20
#define IR3 40
unsigned char temp = 0;

unsigned char MODE = 0;//0基础2函数1音乐
unsigned char mode = 0;
unsigned char method = 0;//0使用P2.2
unsigned char method1_flag = 1;
unsigned char auto_control = 0;

// 数码管位和指示灯显示数据变量

unsigned char NumOfFun = 4;  //有4种波形，正弦、三角、锯齿、方波
unsigned char NumOfMus = 10;  //有10首歌曲

unsigned char output_sel;
// 数码管段显示数据变量
unsigned char output_8seg;

// 1s软件定时器计数
unsigned char clock1s=0;
// 1s软件定时器溢出标志
unsigned char clock1s_flag=0;

// 指示灯驱动信号输出缓存
unsigned char led1,led2,led3,led4;
unsigned int time1,time2,time3,time4;
unsigned char ratio;
unsigned char phase = 0;
unsigned char IR = 0;
unsigned char k = 10;
unsigned char fre = 1;
// 数码管扫描驱动指针
unsigned char digi_scaner;
// 测试用计数器
unsigned int test_counter=0;
// 测试用计数值十进制表示
unsigned char digi[4]={0,0,0,0};

//乐曲的乐谱 {频率值，节拍值}  const类型指明要存放在ROM中
//以下包含9首音乐
const unsigned int music_data0[][2]=		//《莫斯科郊外的晚上》
{
	{440,400},{523,400},{659,400},{523,400},
	{587,800},{523,400},{494,400},{659,800},
	{587,800},{440,1600},{523,400},{659,400},
	{784,400},{784,400},{880,800},{784,400},
	{698,400},{659,1600},{740,800},{831,800},
	{988,400},{880,400},{659,1200},{494,800},{440,400},
	{659,400},{587,400},{698,1600},{784,400},
	{698,400},{659,800},{587,400},{523,400},{659,800},
	{587,800},{440,1600},{740,800},{831,800},
	{988,400},{880,400},{659,1200},{494,800},{440,400},
	{659,400},{587,400},{698,1600},{784,400},
	{698,400},{659,800},{587,400},{523,400},{659,800},
	{587,800},{440,3200},{0,0}
};
const unsigned int music_data1[][2]=		//《梁祝》
{
	{247,800},{294,600},{330,200},{392,600},
	{440,200},{330,200},{392,200},{294,200},
	{0,200},{587,600},{784,200},{659,200},
	{587,200},{494,200},{587,200},{440,1400},
	{0,200},{440,400},{440,200},{494,200},
	{370,400},{330,400},{294,600},{330,200},{392,400},
	{440,400},{247,400},{392,400},{330,200},
	{294,200},{330,200},{392,200},{294,1400},{0,200},
	{494,600},{587,200},{370,400},{440,400},
	{330,200},{392,200},{294,1200},{247,300},{294,100},
	{247,400},{294,300},{330,100},{370,200},
	{440,200},{330,1200},{294,200},{330,200},{392,600},
	{440,200},{587,400},{494,400},{440,400},{494,200},
	{440,200},{392,400},{330,200},{294,200},
	{247,400},{0,400},{392,400},{0,400},{330,300},
	{392,100},{330,200},{294,200},{247,200},{294,200},
	{330,200},{392,200},{294,1400},
	{0,0}
};
const unsigned int music_data2[][2]=			//《青花瓷》
{
	{1568,200},{1568,200},{1319,200},{1175,200},{1319,200},{880,400},{0,200},
	{1175,200},{1319,200},{1568,200},{1319,200},{0,200},{1175,800},{0,200},
	{1568,200},{1569,200},{1319,200},{1175,200},{1319,200},{784,400},{1175,200},
	{0,200},{1047,800},{1047,200},{1175,200},{1319,200},{1568,200},{1760,200},{1569,200},
	{1319,200},{1568,200},{1319,200},{1319,200},{1175,200},{1175,800},{1047,200},{1175,200},
	{1047,200},{1175,400},{1047,200},{1175,200},{1175,200},{1319,400},{1568,200},{1319,200},
	{1319,400},{1568,200},{1568,200},{1319,200},{1175,200},{1319,200},{880,400},{0,200},
	{1175,200},{1319,200},{1568,200},{1319,200},{0,200},{1175,800},{0,200},
	{1568,200},{1569,200},{1319,200},{1175,200},{1319,200},{784,400},{1175,200},
	{0,200},{1047,800},{1047,200},{1175,200},{1319,200},{1568,200},{1760,200},{1569,200},
	{1319,200},{1568,200},{1319,200},{1319,200},{1175,200},{1175,400},{784,200},{1319,400},
	{1175,200},{1175,200},{1175,400},{1047,400},{1047,800},{0,0}
};
const unsigned int music_data3[][2]=	//《超级玛丽》
{
	{659,200},{659,200},{0,200},{659,200},
	{0,200},{523,200},{659,200},{0,200},
	{784,400},{0,400},{392,400},{0,400},
	{523,200},{0,400},{392,400},{0,400},
	{330,200},{0,400},{440,200},{0,200},
	{494,200},{0,200},{466,200},{440,200},{0,200},
	{392,266},{659,266},{784,266},{880,200},
	{0,200},{698,200},{784,200},{0,200},
	{659,200},{0,200},{523,200},{587,200},
	{494,200},{0,400},{523,200},{0,400},{392,400},{0,400},
	{330,200},{0,400},{440,200},{0,200},
	{494,200},{0,200},{466,200},{440,200},{0,200},
	{392,266},{659,266},{784,266},{880,200},
	{0,200},{698,200},{784,200},{0,200},
	{659,200},{0,200},{523,200},{587,200},
	{494,200},{0,800},{784,200},{740,200},
	{698,200},{622,200},{0,200},{659,200},{0,200},
	{415,200},{440,200},{523,200},{0,200},
	{440,200},{523,200},{587,200},{0,400},
	{784,200},{740,200},{698,200},{622,200},{0,200},
	{659,200},{0,200},{1047,200},{0,200},{1047,200},{1047,200},
	{0,800},{784,200},{740,200},
	{698,200},{622,200},{0,200},{659,200},{0,200},
	{415,200},{440,200},{523,200},{0,200},
	{440,200},{523,200},{587,200},{0,200},{622,200},{0,400},{587,200},
	{0,400},{523,200},{0,600},{392,200},{392,200},{0,200},{262,200},{0,400},
	{784,200},{740,200},
	{698,200},{622,200},{0,200},{659,200},{0,200},
	{415,200},{440,200},{523,200},{0,200},
	{440,200},{523,200},{587,200},{0,400},
	{784,200},{740,200},{698,200},{622,200},{0,200},
	{659,200},{0,200},{1047,200},{0,200},{1047,200},{1047,200},
	{0,800},{784,200},{740,200},
	{698,200},{622,200},{0,200},{659,200},{0,200},
	{415,200},{440,200},{523,200},{0,200},
	{440,200},{523,200},{587,200},{0,200},{622,200},{0,400},{587,200},
	{0,400},{523,200},{0,600},{392,200},{392,200},{0,200},{262,200},{0,400},{0,0}
};
const unsigned int music_data4[][2]=		//《星之所在》
{
	{1047,400},{988,400},{1047,400},{1319,400},{988,800},{0,800},{880,400},{784,400},
	{1760,400},{523,400},{784,800},{0,800},{698,400},{1319,400},{698,400},{523,400},{988,800},
	{784,800},{880,400},{988,400},{1047,400},{1319,400},{1175,800},{0,800},{1047,400},{988,400},
	{1047,400},{1319,400},{988,800},{784,800},{880,400},{988,400},{1047,400},{1175,400},{1319,800},
	{1319,800},{1397,400},{1319,400},{1175,400},{1047,400},{988,400},{1319,400},{0,0}
};
const unsigned int music_data5[][2]=		//《机器猫》
{
	{0,800},{393,360},{523,120},{523,360},{659,120},{880,360},{659,120},
	{784,460},{0,20},{784,360},{880,120},{784,360},{650,120},{698,360},
	{659,120},{587,460},{0,20},{440,360},{587,120},{587,360},{698,120},
	{987,360},{987,120},{880,360},{784,120},{698,340},{0,20},{698,120},
	{698,360},{659,120},{440,480},{494,480},{523,960},{0,0}
};
const unsigned int music_data6[][2]=		//《葫芦娃》
{
	  {523,400},{523,400},{659,800},
	  {523,200},{523,400},{659,1000},
	  {880,400},{880,400},{880,200},{784,200},{880,400},
	  {784,200},{523,400},{659,1000},
	  {1046,200},{880,200},{880,200},{784,200},{880,800},
	  {784,200},{523,400},{587,600},{0,400},
	  {987,1000},{784,200},{659,200},{784,1800},
	  {1046,200},{0,200},{880,300},{880,100},{784,300},{784,100},{880,300},{880,100},
	  {0,200},{784,400},{523,200},{659,500},{0,100},
	  {1046,200},{0,200},{880,300},{880,100},{784,300},{784,100},{880,300},{880,100},
	  {0,200},{784,400},{523,200},{587,500},{0,100},
	  {659,1000},{523,200},{440,400},{523,1600},
	  {659,200},{784,400},{880,1000},
	  {659,200},{784,400},{880,1000},
	  {1046,800},{987,400},{784,800},{880,1200},
	  {0,800},
	  {0,0}
};
const unsigned int music_data7[][2]=		//《喀秋莎》
{
	{440,600},{494,200},{523,600},{440,200},
	{523,400},{494,200},{440,200},{494,400},
	{330,400},{494,600},{523,200},{578,600},
	{494,200},{578,400},{523,200},{494,200},{440,800},{659,400},{880,400},{784,400},
	{880,200},{784,200},{698,400},{659,200},
	{578,200},{659,400},{440,400},{0,200},
	{698,400},{578,200},{659,600},{523,200},
	{494,200},{330,200},{523,200},{494,200},
	{440,800},{659,400},{880,400},{784,400},
	{880,200},{784,200},{698,400},{659,200},
	{578,200},{659,400},{440,400},{0,200},
	{698,400},{578,200},{659,600},{523,200},
	{494,200},{330,200},{523,200},{494,200},
	{440,800},{0,0}
};

const unsigned int music_data8[][2]=        //《荷塘月色》
{
		{523,600},{784,200},{523,200},{784,200},{523,200},{587,200},{659,1600},
		{523,600},{784,200},{523,200},{784,200},{523,200},{587,200},{587,1600},
		{523,600},{784,200},{523,200},{784,200}, {587,200} ,{523,200},{440,1000},{392,200}, {523,200},{587,200},
		{523,600},{784,200},{523,200},{784,200},{523,200},{440,200},{523,1600},
		{523,200},{523,400},{440,200},{392,400},{440,400},
		{523,400},{523,200},{587,200},{659,800},
		{587,200},{587,400},{523,200},{587,400},{587,200},{784,200},
		{784,200},{659,200},{659,200},{587,200},{659,800},
		{523,200},{523,400},{440,200},{392,400},{784,400},
		{659,200},{587,200}, {659,200},{587,200},{523,800},
		{587,200}, {587,400}, {523,200}, {587,200}, {587,400},{659,200},
		{587,200},{523,200},{440,200}, {587,200},{523,800},
		{523,200},{523,400},{440,200},{392,400},{440,400},
		{523,200},{523,400},{587,200},{659,800},
		{587,200},{587,400},{523,200},{587,400},{587,200},{784,200},
		{784,200},{659,200},{659,200},{587,200},{659,800},
		{523,200},{523,200},{523,200},{440,200},{392,400},{784,400},
		{659,200},{587,200}, {659,200},{587,200},{523,800},
		{587,200},{587,400}, {523,200}, {587,200}, {587,400},{659,200},
		{587,200},{523,200},{440,200}, {587,200},{523,800},
		{659,200},{784,400}, {784,200}, {784,400}, {784,400},
		{880,200},{784,200},{659,200},{587,200},{523,800},
		{880,200},{1047,200},{880,200},{784,200},{659,200},{587,200},{523,200},{440,200},
		{587,400},{587,200},{659,200},{659,200},{587,600},
		{659,200},{784,400}, {784,200}, {784,400}, {784,400},
		{880,200},{784,200},{659,200},{587,200},{523,800},
		{440,200},{523,200},{440,200},{392,200},{587,400},{659,400},
		{523,1200},{0,400},
		{0,0}

  };

  const unsigned int music_data9[][2] =                 //卡农Canon
{
		{784,400},{659,200},{698,200},{784,400},{659,200},{698,200},
		{784,200},{392,200},{440,200},{493,200},{523,200},{587,200},{659,200},{698,200},
		{659,400},{523,200},{587,200},{659,400},{330,200},{349,200},
		{392,200},{440,200},{392,200},{349,200},{392,200},{523,200},{493,200},{523,200},
		{440,400},{523,200},{493,200},{440,400},{392,200},{349,200},
		{392,200},{349,200},{330,200},{349,200},{392,200},{440,200},{493,200},{523,200},
		{440,400},{523,200},{493,200},{523,400},{493,200},{523,200},
		{493,200},{440,200},{493,200},{523,200},{587,200},{659,200},{698,200},{784,200},
		{784,400},{659,200},{698,200},{784,400},{659,200},{698,200},
		{784,200},{392,200},{440,200},{493,200},{523,200},{587,200},{659,200},{698,200},
		{659,400},{523,200},{587,200},{659,400},{330,200},{349,200},
		{392,200},{440,200},{392,200},{349,200},{392,200},{523,200},{493,200},{523,200},
		{440,400},{523,200},{493,200},{440,400},{392,200},{349,200},
		{392,200},{349,200},{330,200},{349,200},{392,200},{440,200},{493,200},{523,200},
		{440,400},{523,200},{493,200},{523,400},{493,200},{440,200},
		{493,200},{523,200},{587,200},{523,200},{493,200},{523,200},{440,200},{493,200},
		{523,1600},
		{0,0}

  };

/* 幅度调制信号发生波形预定义 */
//由于单片机计算能力的限制，将sin函数计算成数组，查表输出
unsigned int sin[] = {0,16,31,46,59,71,81,90,96,99,100,99,96,90,81,71,59,46,31,16};
unsigned int tri[] = {0,10,20,30,40,50,60,70,80,90,100,90,80,70,60,50,40,30,20,10};
unsigned int saw[] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95};
unsigned int squ[] = {100,100,100,100,100,100,100,100,100,100,0,0,0,0,0,0,0,0,0,0};
unsigned char sin_len = 19;
unsigned char tri_len = 19;
unsigned char saw_len = 19;
unsigned char squ_len = 19;

/* 播放乐曲功能变量 */
// 播放中，当前的音频频率
unsigned int audio_frequency;
// 辅助读谱指针、持续时间计数变量
unsigned int audio_ptr=0,audio_dura=0;
unsigned int const (*music_data)[2];
unsigned int music_num = 0;                 //当前播放歌曲编号

//本程序时钟采用内部RC振荡器。     DCO：8MHz,供CPU时钟;  SMCLK：1MHz,供定时器时钟
#include <msp430g2553.h>
#include <tm1638.h>  //与TM1638有关的变量及函数定义均在该H文件中

//&:0 |:1

#define CTL0_L P1OUT &= ~BIT0
#define CTL0_H P1OUT |= BIT0
#define CTL1_L P1OUT &= ~BIT1
#define CTL1_H P1OUT |= BIT1
#define CTL2_L P1OUT &= ~BIT2
#define CTL2_H P1OUT |= BIT2
#define CTL3_L P1OUT &= ~BIT3
#define CTL3_H P1OUT |= BIT3
#define CTL4_L P1OUT &= ~BIT5
#define CTL4_H P1OUT |= BIT5
#define CTL5_L P1OUT &= ~BIT6
#define CTL5_H P1OUT |= BIT6
#define CTL6_L P1OUT &= ~BIT7
#define CTL6_H P1OUT |= BIT7
#define CTL7_L P2OUT &= ~BIT0
#define CTL7_H P2OUT |= BIT0

//////////////////////////////
//         常量定义         //
//////////////////////////////

// 0.1s软件定时器溢出值，5个20ms
#define V_T100ms	5
// 0.5s软件定时器溢出值，25个20ms
#define V_T500ms	25

//171行没有使用GAIN_STATENUM
#define GAIN_STATENUM 15;

//////////////////////////////
//       变量定义           //
//////////////////////////////

// 软件定时器计数
unsigned char clock100ms=0;
unsigned char clock500ms=0;
// 软件定时器溢出标志
unsigned char clock100ms_flag=0;
unsigned char clock500ms_flag=0;


//////////////////////////////
//       系统初始化         //
//////////////////////////////

void ADC10_Init(void)//ADC初始化子程序，用于自动增益

{
      ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + REF2_5V+ ADC10ON;//Vref+ =2.5V
        P1DIR &= ~BIT4;
        ADC10CTL1 = INCH_4 ;
        ADC10AE0 |= BIT4;
}

//  定时器TIMER0初始化，循环定时20ms
void TIMER0_A0_INIT(void)			//5ms
{
	TA0CTL = TASSEL_2 + MC_1 ;      // Source: SMCLK=1MHz, UP mode,
	TA0CCR0 = T_A0*100;             // 1MHz时钟,计满500次为5ms，T_A0为50
	TA0CCTL0 = CCIE;                // TA0CCR0 interrupt enabled
}

void TIMER0_A1_INIT(void)
{
// Configure TimerA1
    TA1CTL = TASSEL_2 + MC_1 ;          // Source: SMCLK=1MHz, PWM mode
    TA1CCTL1 = OUTMOD_7;
    TA1CCR0 = 100000/440;				//设定周期 ~= 250, 2.5ms
    TA1CCR1 = TA1CCR0;					//设置占空比等于50%

    TA1CCTL0 = CCIE;
}

//  I/O端口和引脚初始化
void Init_Ports(void)
{
	P2SEL &= ~(BIT7+BIT6);       		//P2.6、P2.7 设置为通用I/O端口
	P2DIR |= BIT7 + BIT6 + BIT5; 		//P2.5、P2.6、P2.7 设置为输出

	P1DIR |= BIT0 + BIT1 + BIT2 + BIT3 + BIT5 + BIT6 + BIT7;
	P2DIR |= BIT0;

	//P2.2 设置为定时器 A1的TA1.1　PWM输出用于音乐播放
	P2SEL |= BIT2;
	P2DIR |= BIT2;
	P2DIR &= ~BIT4;
 }


//  MCU器件初始化，注：会调用上述函数
void Init_Devices(void)
{
	WDTCTL = WDTPW + WDTHOLD;     // Stop watchdog timer，停用看门狗
	if (CALBC1_8MHZ ==0xFF || CALDCO_8MHZ == 0xFF)
	{
		while(1);            // If calibration constants erased, trap CPU!!
	}

    //设置时钟，内部RC振荡器。     DCO：8MHz,供CPU时钟;  SMCLK：1MHz,供定时器时钟
	BCSCTL1 = CALBC1_8MHZ; 	 // Set range
	DCOCTL = CALDCO_8MHZ;    // Set DCO step + modulation
	BCSCTL3 |= LFXT1S_2;     // LFXT1 = VLO
	IFG1 &= ~OFIFG;          // Clear OSCFault flag
	BCSCTL2 |= DIVS_3;       //  SMCLK = DCO/8

    Init_Ports();           //调用函数，初始化I/O口

    TIMER0_A0_INIT();          //调用函数，初始化定时器0
    TIMER0_A1_INIT();

	ADC10_Init();
    _BIS_SR(GIE);           //开全局中断
   //all peripherals are now initialized
}

void gain_control(void)
{
	switch (gain_state){
	case 1:  CTL3_H; CTL2_L; CTL1_L; CTL0_L; break;
	case 2:  CTL3_L; CTL2_H; CTL1_L; CTL0_L; break;
	case 3:  CTL3_H; CTL2_H; CTL1_L; CTL0_L; break;
	case 4:  CTL3_L; CTL2_L; CTL1_H; CTL0_L; break;
	case 5:  CTL3_H; CTL2_L; CTL1_H; CTL0_L; break;
	case 6:  CTL3_L; CTL2_H; CTL1_H; CTL0_L; break;
	case 7:  CTL3_H; CTL2_H; CTL1_H; CTL0_L; break;
	case 8:  CTL3_L; CTL2_L; CTL1_L; CTL0_H; break;
	case 9:  CTL3_H; CTL2_L; CTL1_L; CTL0_H; break;
	case 10: CTL3_L; CTL2_H; CTL1_L; CTL0_H; break;
	case 11: CTL3_H; CTL2_H; CTL1_L; CTL0_H; break;
	case 12: CTL3_L; CTL2_L; CTL1_H; CTL0_H; break;
	case 13: CTL3_H; CTL2_L; CTL1_H; CTL0_H; break;
	case 14: CTL3_L; CTL2_H; CTL1_H; CTL0_H; break;
	case 15: CTL3_H; CTL2_H; CTL1_H; CTL0_H; break;
	}
}

void wave_control(void)
{
    if (ratio > 127) {
        CTL0_H;
        ratio -= 128;
    }
    else CTL0_L;
    if (ratio > 63){
        CTL1_H;
        ratio -= 64;
    }
    else CTL1_L;
    if (ratio > 31){
        CTL2_H;
        ratio -= 32;
    }
    else CTL2_L;
    if (ratio > 15){
        CTL3_H;
        ratio -= 16;
    }
    else CTL3_L;
    if (ratio > 7){
        CTL4_H;
        ratio -= 8;
    }
    else CTL4_L;
    if (ratio > 3){
        CTL5_H;
        ratio -= 4;
    }
    else CTL5_L;
    if (ratio > 1){
        CTL6_H;
        ratio -= 2;
    }
    else CTL6_L;
    if (ratio > 0){
        CTL7_H;
        ratio -= 1;
    }
    else CTL7_L;
}

void Play_Music(void)
{
	if (audio_dura==0)
	{
	    TA1CTL = 0;
	    if (music_data[audio_ptr][1]==0)   //乐曲结束标志{0,0}
	    {
	        audio_ptr=0;
	        audio_dura=0;  //准备重新播放
	    }
	    else
	    {
	        audio_dura=music_data[audio_ptr][1]/10;    //改变除数可调节快慢
	        if (music_data[audio_ptr][0]!=0)
	        {
	            audio_frequency = music_data[audio_ptr][0];
	            TA1CCR0 = 1000000/audio_frequency;  //设定周期
                TA1CCR1 = TA1CCR0/2;                //设置占空比等于50%
                TA1CTL = TASSEL_2 + MC_1;           //Source: SMCLK=1MHz, PWM mode
                audio_ptr++;                        //指向下一个音符
	        }
	        else audio_ptr++;
	    }
	}
	else audio_dura--;
}



//////////////////////////////
//      中断服务程序        //
//////////////////////////////

// Timer0_A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void)
{
	if(auto_control)
	{
		//__disable_interrupt();
		ADC10CTL0 |= ENC + ADC10SC;
		while (ADC10CTL1 & ADC10BUSY);//等待ad转换器
		sample = ADC10MEM;
		volt = sample*2.5/1024;

		if(volt > Vmax && gain_state >1) --gain_state;
		if(volt < Vmin && gain_state <15) ++gain_state;

		ADC10CTL0 |= ENC + ADC10SC;
		//__enable_interrupt();
	}

	//红外脉冲检测
	input2_4 = (P2IN & BIT4) >> 4;
	if (input_status == 0){
		if (input2_4 == 1){
			++ input_adder;
		}
		else if (input_adder >= short1*10/T_A0 && input_adder <= short2*10/T_A0) input_status = 1;
		else if (input_adder >= long1*10/T_A0 && input_adder <= long2*10/T_A0) input_status = 2;
		else{
			input_status = 0;
			if (input_adder > long2*10/T_A0) input_adder = 0;
		}
	}

	if (++clock100ms>=V_T100ms)
	{
		clock100ms_flag = 1; //当0.1秒到时，溢出标志置1
		clock100ms = 0;
	}

//	if (MODE == 0){
//
//	}
//幅度调制信号发生器，波形在前面常量定义
	else if (MODE == 2){
		switch (mode)
		{
		case 0:
			if (phase>sin_len) phase = 0;
			ratio = (int)(number * sin[phase] / 100 * gain_state / 15);
			break;
		case 1:
			if (phase>tri_len) phase = 0;
			ratio = (int)(number * tri[phase] / 100 * gain_state / 15);
			break;
		case 2:
			if (phase>saw_len) phase = 0;
			ratio = (int)(number * saw[phase] / 100 * gain_state / 15);
			break;
		case 3:
			if (phase>squ_len) phase = 0;
			ratio = (int)(number * squ[phase] / 100 * gain_state / 15);
			break;
		}
		phase++;
		wave_control();
	}

	else if (MODE == 1){
    	Play_Music();
	}

	// 刷新全部数码管和LED指示灯
	TM1638_RefreshDIGIandLED(digit,pnt,led);

	// 检查当前键盘输入，0代表无键操作，1-16表示有对应按键
	//   键号显示在两位数码管上
	key_code=TM1638_Readkeyboard();

	switch (key_state)
	{
	case 0:
		if (key_code > 0) {key_state = 1;key_flag = 1;}
		break;
	case 1:
		if (key_code == 0) {key_state = 0;}
		break;
	default:
		break;
	}
}

// Timer0_A1 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0 (void)
{
    if (method == 1) {
    	if (method1_flag == 1) {
    		CTL0_H;
    		method1_flag = 0;
    	}
    	else {
    		CTL0_L;
    		method1_flag = 1;
    	}
    }

}

//////////////////////////////
//         主程序           //
//////////////////////////////

int main(void)
{
	//unsigned char i=0,temp;
	Init_Devices( );
	while (clock100ms<3);   // 延时60ms等待TM1638上电完成
	init_TM1638();	    //初始化TM1638
	music_data = music_data0;

	while(1)//中断未发生的时候，CPU在程序中不断循环
	{

		if (key_flag == 1){
			key_flag = 0;
			switch (key_code)
			{
            case 1:           //按键1：改变模式
                if (++MODE >2) {
                    MODE = 0;
                    TA1CTL = 0;
                }
                break;
            case 2:           //按键2：改变波形或音乐
                if (MODE == 2){
                	if (++mode >= NumOfFun) mode = 0;
                }
                if (MODE == 1){
                	++music_num;
                	music_num = music_num%NumOfMus;
                	mode = music_num;
                	switch (music_num)           //循环切歌
                	{
                		case 0: music_data = music_data0; audio_dura=0; audio_ptr=0; audio_frequency=0; break;
                	    case 1: music_data = music_data1; audio_dura=0; audio_ptr=0; audio_frequency=0; break;
                	    case 2: music_data = music_data2; audio_dura=0; audio_ptr=0; audio_frequency=0; break;
                	    case 3: music_data = music_data3; audio_dura=0; audio_ptr=0; audio_frequency=0; break;
                	    case 4: music_data = music_data4; audio_dura=0; audio_ptr=0; audio_frequency=0; break;
                	    case 5: music_data = music_data5; audio_dura=0; audio_ptr=0; audio_frequency=0; break;
                	    case 6: music_data = music_data6; audio_dura=0; audio_ptr=0; audio_frequency=0; break;
                	    case 7: music_data = music_data7; audio_dura=0; audio_ptr=0; audio_frequency=0; break;
                	    case 8: music_data = music_data8; audio_dura=0; audio_ptr=0; audio_frequency=0; break;
                	    case 9: music_data = music_data9; audio_dura=0; audio_ptr=0; audio_frequency=0; break;
                	}
                }
                    break;
//            case 7:
//                if (MODE == 1){
//                    if (method == 0) method = 1;
//                    else method = 0;
//                }
//                break;
            case 8:      //按键8：打开或关闭自动增益
            	if (auto_control == 1)auto_control = 0;
            	else auto_control = 1;
            	break;
            case 5:      //按键5,6改变频率
            	if (++T_A0 > 99) T_A0 = 1;
            	TA0CCR0 = T_A0*100;
            	break;
            case 6:
            	if (--T_A0 < 1) T_A0 = 99;
            	TA0CCR0 = T_A0*100;
            	break;
            case 4:      //按键3,4改变增益
                if (++gain_state > 15) gain_state = 1;
                break;
            case 3:
                if (--gain_state < 1) gain_state = 15;
                break;
            default:
                break;
			}
		}            //红外信号处理
		if (input_status != 0){
			if (input_status == 1){
				if (--gain_state < 1) gain_state = 15;
			}
			if (input_status == 2){
				if (++gain_state > 15) gain_state = 1;
			}
			input_adder = 0;
			input_status = 0;
		}
        //增益
		if (MODE == 0 || MODE ==1 )gain_control();

		digit[0] = (100-T_A0)/10;
		digit[1] = (100-T_A0)%10;//定时器定时中断周期调节
		digit[2] = gain_state/10;
		digit[3] = gain_state%10;//增益
		digit[4] = MODE;//0-基础，1-音乐，2-幅度调制
		digit[5] = mode;//第n种波形 or 第n首曲子
		//digit[6] = method;
		digit[7] = auto_control;//自动增益控制，0-OFF，1—ON
	}
}