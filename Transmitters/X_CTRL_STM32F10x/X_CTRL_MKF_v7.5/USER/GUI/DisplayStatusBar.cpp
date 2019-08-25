#include "FileGroup.h"
#include "GUI_Private.h"
#include "ComPrivate.h"

/*״̬��ʱ��Ƭ���������������ȼ�*/
static MillisTaskManager mtm_StatusBar(3, true);

/*��ص�����ʾ�ؼ�*/
static LightGUI::ProgressBar<SCREEN_CLASS> PB_Batt(&screen, 0);

/*CPUռ������ʾʹ��*/
bool State_DisplayCPU_Usage = false;

/*�ı�Y�������*/
#define TextPosBase_Y (StatusBar_POS-15)

/**
  * @brief  ��ص�����ʾ��������
  * @param  ��
  * @retval ��
  */
static void Task_DrawBattUsage()
{
    static uint8_t usage;//��ʾ�ĵ�ص���
    bool IsBattCharging = !digitalRead(BattCharge_Pin);

    __ValueCloseTo(usage, (int)BattUsage, 1);//����ʾ�ĵ�������ʵ�ʵĵ������ﵽ������Ч��
    if(IsBattCharging && usage == (uint8_t)BattUsage)
    {
        usage = 0;
    }

    __ValueMonitor(
        IsBattCharging,
        IsBattCharging ? BuzzMusic(MC_Type::MC_BattChargeStart) : BuzzMusic(MC_Type::MC_BattChanargeEnd)
    );

    /*������ʾͼ��*/
    if(usage < 30)
        PB_Batt.Color_PB = screen.Red;//����<30%�����Ϊ��ɫ
    else if(usage >= 90)
        PB_Batt.Color_PB = screen.Green;//����>90%�����Ϊ��ɫ
    else
        PB_Batt.Color_PB = screen.White;//�������������Ϊ��ɫ

    PB_Batt.setProgress(usage / 100.0);//���¿ؼ�

    /*���ͷ*/
    screen.drawFastVLine(
        PB_Batt.X + PB_Batt.Width,
        PB_Batt.Y + 2,
        PB_Batt.Height - 4,
        screen.White
    );
}

/**
  * @brief  NRF�շ�״̬��ʾ����
  * @param  ��
  * @retval ��
  */
static void Task_DrawNrfTxRxState()
{
    const int16_t PosBase = screen.width() / 2 + 7;
    screen.fillRect(PosBase, StatusBar_POS - 10, 5, 7, screen.Black);
    if(page.NowPage == PAGE_CtrlInfo)
    {
        //screen.drawFastVLine(PosBase + 2, 2, 7, screen.White);
        if(nrf.GetTxRxState() == nrf.RX_MODE)
        {
            screen.drawLine(PosBase + 2, StatusBar_POS - 4, PosBase + 2 - 2, StatusBar_POS - 4 - 2, screen.White);
            screen.drawLine(PosBase + 2, StatusBar_POS - 4, PosBase + 2 + 2, StatusBar_POS - 4 - 2, screen.White);
        }
        else //(nrf.GetTxRxState() == nrf.TX_MODE)
        {
            screen.drawLine(PosBase + 2, StatusBar_POS - 10, PosBase + 2 - 2, StatusBar_POS - 10 + 2, screen.White);
            screen.drawLine(PosBase + 2, StatusBar_POS - 10, PosBase + 2 + 2, StatusBar_POS - 10 + 2, screen.White);
        }
    }
}

/**
  * @brief  1000ms��������
  * @param  ��
  * @retval ��
  */
static void Task_1000msUpdate()
{
    if(State_DisplayCPU_Usage)
    {
        if(CPU_Usage > 90.0f)
            screen.setTextColor(screen.Red, screen.Black);
        else if(CPU_Usage > 50.0f)
            screen.setTextColor(screen.Yellow, screen.Black);
        else if(CPU_Usage > 10.0f)
            screen.setTextColor(screen.White, screen.Black);
        else
            screen.setTextColor(screen.Green, screen.Black);

        screen.setCursor(10, StatusBar_POS + 2);
        screen.printfX("CPU:%0.2f%% %0.1fC", CPU_Usage, CPU_Temperature);
    }

    /* NRF �ź�ǿ��(0~100%) */
    NRF_SignalStrength = nrf.GetSignalStrength();
    screen.setTextSize(1);
    if(NRF_SignalStrength >= 90)
        screen.setTextColor(screen.Green, screen.Black);
    else if(NRF_SignalStrength >= 60)
        screen.setTextColor(screen.White, screen.Black);
    else if(NRF_SignalStrength >= 20)
        screen.setTextColor(screen.Yellow, screen.Black);
    else if(NRF_SignalStrength < 20)
        screen.setTextColor(screen.Red, screen.Black);

    screen.setCursor(2, TextPosBase_Y);
    screen.printfX("%3d%%   ", NRF_SignalStrength);

    /* ��ʾ��������״̬ */
    screen.setCursor(33, TextPosBase_Y);
    if(State_Bluetooth)
    {
        if(hc05.GetState())
            screen.setTextColor(screen.Green, screen.Black);
        else
            screen.setTextColor(screen.Yellow, screen.Black);

        screen.printfX(StrBtc[Bluetooth_ConnectObject]);
    }
    else
        screen.fillRect(33, StatusBar_POS - 16, 22, 16, screen.Black);

    /* ��Ƶͨ��״̬ */
    digitalWrite(InfoLED_Pin, !State_RF);
    if(State_RF)
    {
        if(digitalRead(SPDT_Switch_Pin))
            screen.setTextColor(screen.Green, screen.Black);
        else
            screen.setTextColor(screen.Yellow, screen.Black);
    }
    else
    {
        screen.setTextColor(screen.Red, screen.Black);
    }
    screen.setCursor(screen.width() / 2 - 9, TextPosBase_Y);
    screen.printfX("RF");

    /* ��ʾ��ص�ѹ */
    screen.setCursor(PB_Batt.X - 27, TextPosBase_Y);
    screen.setTextSize(1);
    screen.setTextColor(screen.White, screen.Black);
    screen.printfX("%1.1fV", BattVoltage);
}

/**
  * @brief  ��ʼ��״̬��
  * @param  ��
  * @retval ��
  */
void Init_StatusBar()
{
    /*״̬����£����*/
    for(float i = 0.0f; i < 1.0f; i += 0.001f)
    {
        screen.drawFastHLine(0, StatusBar_POS, screen.width() / 2 * i + 1, screen.White);
        screen.drawFastHLine(screen.width() - screen.width() / 2 * i, StatusBar_POS, screen.width() / 2 * i + 1, screen.White);
    }

    pinMode(BattCharge_Pin, INPUT_PULLUP);
    pinMode(InfoLED_Pin, OUTPUT);

    /*��ص�����ʾ�ؼ���������*/
    PB_Batt.Width = 20;
    PB_Batt.Height = 10;
    PB_Batt.X = screen.width() - PB_Batt.Width - 5;
    PB_Batt.Y = StatusBar_POS - PB_Batt.Height - 2;

    /*����ע��*/
    mtm_StatusBar.TaskRegister(0, Task_DrawBattUsage, 20);
    mtm_StatusBar.TaskRegister(1, Task_1000msUpdate, 1000);
    mtm_StatusBar.TaskRegister(2, Task_DrawNrfTxRxState, 123);
}

/**
  * @brief  ״̬���߳�
  * @param  ��
  * @retval ��
  */
void Thread_StatusBar()
{
    mtm_StatusBar.Running(millis());
}