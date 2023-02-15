# WiseComm

- [WiseComm](#wisecomm)
  - [Introduction](#introduction)
  - [Function使用方式](#function使用方式)
    - [建立變數與連線](#建立變數與連線)
    - [詢問DI狀態](#詢問di狀態)
    - [詢問AI狀態](#詢問ai狀態)
    - [控制Relay (Digital output)](#控制relay-digital-output)
    - [詢問Relay目前狀態](#詢問relay目前狀態)
  - [應用範例](#應用範例)
    - [腳位連接](#腳位連接)
    - [程式碼範例](#程式碼範例)

## Introduction

此函式庫用來控制研華（Advantech）的 WISE-4012E 的2.4G WiFi IoT無線IO模組

![裝置圖片](https://ftp.ical.tw:5001/webapi/entry.cgi/PXL_20201204_010346644_crop.jpg?api=SYNO.SynologyDrive.Files&method=download&version=2&files=%5B%22id%3A590707072685879396%22%5D&force_download=false)

WISE-4012E有 2個AI(Analog Input)、2個DI(Digital Input)、2個RL(Relay)(繼電器) 可以使用，這個library使用他的modbus界面進行控制

## Function使用方式

以下講解通用使用方式

### 建立變數與連線

```cpp
WiseComm wise;
wise.connect(ip_addr, 502);
```

### 詢問DI狀態

```cpp
bool DI0 = wise.readDigitalInput0(); // DI 0 狀態
bool DI1 = wise.readDigitalInput1(); // DI 1 狀態
// 或是
int DI = wise.readDigitalInputAll(); // 所有DI狀態，2個bit表示DI0和1的狀態，參考下方對應表
```

| DI數值 | DI 1 | DI 0 |
| ------ | ---- | ---- |
| 0      | 0    | 0    |
| 1      | 0    | 1    |
| 2      | 1    | 0    |
| 3      | 1    | 1    |


### 詢問AI狀態

library目前尚未支援，因為我現在沒用到，所以還沒寫 :P

### 控制Relay (Digital output)

```cpp
wise.writeRelay0(true);     // RL0 短路
wise.writeRelay1(false);    // RL1 開路
// 也可以寫成一行
wise.writeRelayAll(1);      // 一次控制兩個Relay，兩個bit代表兩個relay的狀態，可參考前一段的對應表
```

### 詢問Relay目前狀態

```cpp
bool DO0 = wise.readRelay0();
bool DO1 = wise.readRelay1();
// 或者是
int DO = wise.readRelayAll();
```

## 應用範例

現在把真空產生器接到WISE-4012E來控制，他開機之後會連線到ASUS_Arm_Router的WiFi，IP已經設定是192.168.0.100。

![連接範例圖](https://ftp.ical.tw:5001/webapi/entry.cgi/PXL_20201204_010346644.jpg?api=SYNO.SynologyDrive.Files&method=download&version=2&files=%5B%22id%3A590707074678173801%22%5D&force_download=false)

### 腳位連接

| 腳位 | 連接目的 |
| ---- | -------- |
| RL0 | 真空產生器的吸氣
| RL1 | 真空產生器的吐氣
| DI0 | 真空偵測器的狀態 0代表真空 1代表沒有真空

### 程式碼範例

完整程式碼可以參考 test_wise_comm_2.cpp 和 test_wise_comm.cpp

以下是重要的片段

```cpp
// 與裝置連線
WiseComm wise;
wise.connect("192.168.0.100", 502);

// 抽真空
wise.writeRelay0(true);
wise.writeRelay1(false);

// 吐氣
wise.writeRelay0(false);
wise.writeRelay1(true);

// 不吸也不吐
wise.writeRelay0(false);
wise.writeRelay1(false);

// 檢測真空狀態
// false或0 代表真空，可能已經吸到東西
// true或1 代表不是真空，目前沒有吸到東西
bool DI0 = wise.readDigitalInput0();
```




