# AI for sensor picochu電流量測開發板
## TSIC推出第一份
* Lab1：按鈕切換狀態
* Lab2：按鈕寫入資料至SD卡
* Lab3：按鈕寫入資料至SD卡並依照狀態命名
* Lab4：按鈕寫入1000筆資料隨機亂數的資料至SD卡並依照狀態命名
* Lab5：按鈕寫入ADC取樣的資料至SD卡並依照狀態命名
* Lab7s:按鈕寫入以1000HzADC取樣的資料至SD卡並依照狀態命名
* sch 存放PCB原理圖、BOM表、座標文件
* Inference 推論程式碼

如果在推論中遇到__PKHTB的錯誤的話需要到```文件\Arduino\libraries\Pico_inferencing\src\edge-impulse-sdk\dsp```中的```config.hpp```進行修改
在
```#endif // Mbed / ARM Core check
#endif // ifndef EIDSP_USE_CMSIS_DSP
```
之後添加
```
#define __PKHBT(ARG1, ARG2, ARG3) ( (((int32_t)(ARG1) << 0) & (int32_t)0x0000FFFF) | \
                                    (((int32_t)(ARG2) << (ARG3)) & (int32_t)0xFFFF0000) )
#define __PKHTB(ARG1, ARG2, ARG3) ( (((int32_t)(ARG1) << 0) & (int32_t)0xFFFF0000) | \
                                    (((int32_t)(ARG2) >> (ARG3)) & (int32_t)0x0000FFFF) )

```
