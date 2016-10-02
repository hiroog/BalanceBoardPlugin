// 2016 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#ifndef	WBLIB_API_H_
#define	WBLIB_API_H_


class WBLibAPI {
public:
	enum : unsigned int {
		STATE_BUSY,
		STATE_DEVICE_NOT_FOUND,
		STATE_CALIBRATION,
		STATE_READY,
		STATE_ADD_DEVICE,
		STATE_WAIT_DEVICE,
	};
	struct BalanceData {
		float	Weight;
		float	Point[4];
	};
public:
	static void		Initialize();
	static void		Finalize();
	static void		Calibration();
	static void		AddDevice();
	static bool		Update();
	static unsigned int	GetState();
	static void		GetData( BalanceData& data );
};


#endif



