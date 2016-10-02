# BalanceBoardPlugin #


Wii Balance Board  UnrealEngine 4 Plugin


## Sample ##

* [Sample Project (Bitbucket)](https://bitbucket.org/hiroog/wbweightmeter2)
* [Sample Project (GitHub)](https://github.com/hiroog/WBWeightMeter2)

* [Native application](https://bitbucket.org/hiroog/wbhealthmeter)


## Build ##

 1. run Setup.bat
 2. build bboardlib/win32/bboardlib.sln (Release + x64)
 3. copy BalanceBoardPlugin/BalanceBoardPlugin to your project 


## Blueprint Functions ##

    void AddBalanceBoard();
    void StartBalanceBoardCalibration();
    void GetBalanceBoardState( TEnumAsByte<EBalanceBoardState>& state );
    bool UpdateBalanceBoard( float& total_weight_KG, float& w0, float& w1, float& w2, float& w3, TEnumAsByte<EBalanceBoardState>& state );



#### AddBalanceBoard ####

Start pairing with BalanceBoard

#### StartBlanaceBoardCalibration ####


#### GetBalanceBoardState ####


#### UpdateBalanceBoard ####
