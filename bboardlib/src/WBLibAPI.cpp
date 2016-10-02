// 2016 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#include	"flCore.h"
#include	"flWiiLib.h"
#include	"flBluetoothLib.h"
#include	"flPlatformThread.h"
#include	"flSimpleMath.h"
#include	"WBLibAPI.h"


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

struct LocalParam {
	enum : unsigned int {
		BALANCE_UNIT_COUNT	=	4,
		CALIBRATION_TIME	=	60,
		STARTUP_TIME		=	30*4,
	};
	float			TotalWeight;
	float			Point[BALANCE_UNIT_COUNT];
	unsigned int	DeviceType;
	unsigned int	AdjustTimer;
	flWiiLibDevice*	iDevice;
public:
	void	Clear()
	{
		TotalWeight= 0.0f;
		DeviceType= 0;
		iDevice= nullptr;
		AdjustTimer= 0;
		Calibration();
	}
	void	Calibration()
	{
		TotalWeight= 0.0f;
		for( unsigned int wi= 0 ; wi< BALANCE_UNIT_COUNT ; wi++ ){
			Point[wi]= 0.0f;
		}
		AdjustTimer= CALIBRATION_TIME;
	}
	void	UpdateDevice( flWiiLibDevice* device )
	{
		device->DecodePacket();
		if( AdjustTimer ){
			device->CalibrationBalance( true );
			AdjustTimer--;
		}else{
			device->CalibrationBalance( false );
		}
		float	total= device->GetWeight();

		TotalWeight= total;
		for( unsigned int wi= 0 ; wi< BALANCE_UNIT_COUNT ; wi++ ){
			Point[wi]= device->GetBalancePressure( wi );
		}
	}
};



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


class WBLib {
	enum : unsigned int {
		LOCAL_PARAM_COUNT	=	32,
	};
	flWiiLib*		iWiiLib;
	flThread		WorkerThread;
	LocalParam*		iCurrentParam;
	unsigned int	State;
	volatile bool	bAddDevice;
	LocalParam		Local[LOCAL_PARAM_COUNT];
public:
	WBLib() :
		iWiiLib( nullptr ),
		iCurrentParam( nullptr ),
		State( WBLibAPI::STATE_BUSY ),
		bAddDevice( false )
	{
	}
	~WBLib()
	{
		Finalize();
	}
	void	Initialize();
	void	Finalize()
	{
		WorkerThread.Quit();
		if( iWiiLib ){
			ZRELEASE( iWiiLib );
		}
	}
	bool	Update();
	void	Calibration()
	{
		for( unsigned int di= 0 ; di< LOCAL_PARAM_COUNT ; di++ ){
			Local[di].Calibration();
		}
	}
	unsigned int	GetState() const
	{
		return	State;
	}
	void	GetData( WBLibAPI::BalanceData& data ) const
	{
		if( iCurrentParam ){
			data.Weight= iCurrentParam->TotalWeight;
			data.Point[0]= iCurrentParam->Point[0];
			data.Point[1]= iCurrentParam->Point[1];
			data.Point[2]= iCurrentParam->Point[2];
			data.Point[3]= iCurrentParam->Point[3];
		}else{
			data.Weight= 0.0f;
			data.Point[0]= 0.0f;
			data.Point[1]= 0.0f;
			data.Point[2]= 0.0f;
			data.Point[3]= 0.0f;
		}
	}
	static void*	AddDeviceFunc( void* arg )
	{
		WBLib*	_This= reinterpret_cast<WBLib*>( arg );
		_This->bAddDevice= true;
		_This->State= WBLibAPI::STATE_ADD_DEVICE;
		flBluetoothLib*	iBluetooth= flBluetoothLib::Create();
		if( iBluetooth ){
			if( iBluetooth->AddDevice( 5 ) ){
				Sleep( 1000 * 10 );
				_This->State= WBLibAPI::STATE_WAIT_DEVICE;
				_This->iWiiLib->Update();
				_This->State= WBLibAPI::STATE_CALIBRATION;
				_This->Calibration();
			}
			iBluetooth->Release();
		}
		_This->bAddDevice= false;
		return	nullptr;
	}
	void	AddDevice()
	{
		WorkerThread.Quit();
		WorkerThread.Init();
		WorkerThread.CreateThread( AddDeviceFunc, this );
	}
};





void	WBLib::Initialize()
{
	iWiiLib= nullptr;
	iCurrentParam= nullptr;

	for( unsigned int li= 0 ; li< LOCAL_PARAM_COUNT ; li++ ){
		Local[li].Clear();
	}


	WorkerThread.Init();

	iWiiLib= flWiiLib::Create();
	iWiiLib->BeginLoop();
}




bool	WBLib::Update()
{
	if( bAddDevice ){
		return	false;
	}

	unsigned int	device_count= iWiiLib->GetDeviceCount();

	if( device_count >= LOCAL_PARAM_COUNT ){
		device_count= LOCAL_PARAM_COUNT;
	}
	iCurrentParam= nullptr;
	bool	busy= false;
	for( unsigned int di= 0 ; di< device_count ; di++ ){
		LocalParam*		lp= &Local[di];
		flWiiLibDevice*	dev= iWiiLib->GetDevice( di );
		lp->iDevice= nullptr;
		if( !dev->IsValid() ){
			continue;
		}
		if( dev->GetReadType() ){
			busy= true;
			continue;
		}
		lp->iDevice= dev;
		unsigned int	type= dev->GetExtensionType();
		lp->DeviceType= type;
		switch( type ){
		case flWiiLibDevice::EXTENSIONTYPE_BALANCE:
			lp->UpdateDevice( dev );
			iCurrentParam= lp;
			break;
		}
	}
	if( busy ){
		State= WBLibAPI::STATE_BUSY;
		return	false;
	}
	if( !iCurrentParam ){
		State= WBLibAPI::STATE_DEVICE_NOT_FOUND;
		return	false;
	}
	if( iCurrentParam->AdjustTimer ){
		State= WBLibAPI::STATE_CALIBRATION;
		return	false;
	}
	State= WBLibAPI::STATE_READY;
	return	true;
}



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static WBLib	Lib;


void	WBLibAPI::Initialize()
{
	Lib.Initialize();
}


void	WBLibAPI::Finalize()
{
	Lib.Finalize();
}


void	WBLibAPI::Calibration()
{
	Lib.Calibration();
}

void	WBLibAPI::AddDevice()
{
	Lib.AddDevice();
}


bool	WBLibAPI::Update()
{
	return	Lib.Update();
}


unsigned int	WBLibAPI::GetState()
{
	return	Lib.GetState();
}



void	WBLibAPI::GetData( BalanceData& data )
{
	Lib.GetData( data );
}


