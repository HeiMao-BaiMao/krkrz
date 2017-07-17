/**
 * まずはスレッドとして実装したが、実際はタイマー動作か……
 * タイマーを継承して実装で十分かな描画待ちをするわけじゃないし
 * ただ、負荷が高くなるとイベントが詰まる
 */

#include "tjsCommHead.h"

#include "DrawCycleTimer.h"
#include "WindowIntf.h"
#include "DebugIntf.h"
#include "MsgIntf.h"
#include "TickCount.h"
#include "TimerIntf.h"

//---------------------------------------------------------------------------
tTVPDrawCycleTimer::tTVPDrawCycleTimer(tTJSNI_BaseWindow* owner) : OwnerWindow(owner) {
	tTVPTimerThread::Add(this);
}
//---------------------------------------------------------------------------
tTVPDrawCycleTimer::~tTVPDrawCycleTimer() {
	tTVPTimerThread::Remove(this);
	ZeroPendingCount();
}
//---------------------------------------------------------------------------
void tTVPDrawCycleTimer::Fire(tjs_uint n) {
	if( OwnerWindow == nullptr ) return;

	OwnerWindow->StartDrawingInternal();

	tjs_uint32 tick = TVPGetRoughTickCount32();
	tjs_uint32 lasttick = LastDrawTick;
	if( lasttick != 0 ) {
		tjs_uint32 drawInterval = 1000/DrawCycle;
		tjs_uint32 interval;
		if( lasttick < tick ) {
			interval = tick - lasttick;
		} else {
			interval = (tjs_uint32)( ((((tjs_uint64)lasttick)+0xFFFFFFFFULL) - ((tjs_uint64)tick))&0xFFFFFFFF );
		}
		if( interval > ((drawInterval*3)/2) ) {
			// 描画サイクルの1.5倍より遅れている、混みあっているので、タイマー再設定して遅らせる
			ResetDrawCycle();
			LastDrawTick = 0;
		}
	} else {
		LastDrawTick = tick;
	}
}
//---------------------------------------------------------------------------
void tTVPDrawCycleTimer::ResetDrawCycle()
{
	if( DrawCycle != 0 ) {
		SetEnabled( false );
		SetEnabled( true );
	}
}
//---------------------------------------------------------------------------
void tTVPDrawCycleTimer::SetDrawCycle( tjs_uint32 cycle ) {
	if( DrawCycle != cycle ) {
		bool enable = cycle == 0;
		bool prevEnable = DrawCycle != 0;
		if( prevEnable ) SetEnabled( false );
		DrawCycle = cycle;
		if( cycle != 0 ) {
			SetInterval( ( 1000 << TVP_SUBMILLI_FRAC_BITS ) / cycle );
		}
		if( !prevEnable || !enable ) {
			SetEnabled( enable );
		}
		LastDrawTick = 0;
	}
}
//---------------------------------------------------------------------------
