#include <stdio.h>
#include <tchar.h>
#include <locale.h>
#include <iostream>
#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <wincodecsdk.h>

// lib
#pragma comment( lib, "d2d1.lib" )
#pragma comment( lib, "WindowsCodecs.lib" )

// 外部宣言
LRESULT CALLBACK eMainWindowProc(
	HWND   hWnd,  // handle to window
    UINT   uMsg,   // message identifier
    WPARAM wParam, // first message parameter
    LPARAM lParam // second message parameter
);

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if(p)      { (p)->Release(); (p)=NULL; } }
#endif




class D2DtestClass
{
	
	// グローバル変数
	IWICImagingFactory*		pWICImagingFactory = NULL;

	ID2D1Factory*			pD2d1Factory = NULL;
	ID2D1HwndRenderTarget*	pRenderTarget = NULL;
	

	public:
	bool	WindowProcCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
	{
		CREATESTRUCT* tpCreateSt = (CREATESTRUCT*)lParam;
	 
		/* パラメータ表示 */
		wprintf(
		    L"CREATESTRUCT¥n"
		    L"¥tlpCreateParams = 0x%08x¥n"
		    L"¥thInstance      = 0x%08x¥n"
		    L"¥thMenu          = 0x%08x¥n"
		    L"¥thwndParent     = 0x%08x¥n"
		    L"¥tcy             = %d¥n"
		    L"¥tcx             = %d¥n"
		    L"¥ty              = %d¥n"
		    L"¥tx              = %d¥n"
		    L"¥tstyle          = 0x%08x¥n"
		    L"¥tlpszName       = %s¥n"
		    L"¥tlpszClass      = %s¥n"
		    L"¥tdwExStyle      = 0x%08x¥n"
		    , tpCreateSt->lpCreateParams
		    , tpCreateSt->hInstance
		    , tpCreateSt->hMenu
		    , tpCreateSt->hwndParent
		    , tpCreateSt->cy
		    , tpCreateSt->cx
		    , tpCreateSt->y
		    , tpCreateSt->x
		    , tpCreateSt->style
		    , tpCreateSt->lpszName
		    , tpCreateSt->lpszClass
		    , tpCreateSt->dwExStyle
		);
		
		HRESULT hResult = S_OK;
		
		// IWICImagingFactoryの生成
		hResult = ::CoCreateInstance( CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<void **>( &pWICImagingFactory ) );
		if ( FAILED( hResult ) )
		{   std::wcout << L"CoCreateInstance失敗" << std::endl;
		    return false;	// default
		}

		// ID2D1Factoryの生成
		hResult = ::D2D1CreateFactory( D2D1_FACTORY_TYPE_MULTI_THREADED, &pD2d1Factory );
		if( FAILED( hResult ) )
		{	std::wcout << L"D2D1CreateFactory失敗" << std::endl;
			return false;	// default
		}
		
		// ID2D1HwndRenderTargetの生成
		{	D2D1_SIZE_U oPixelSize ={ tpCreateSt->cx, tpCreateSt->cy };		
		    D2D1_RENDER_TARGET_PROPERTIES oRenderTargetProperties = D2D1::RenderTargetProperties();
		    D2D1_HWND_RENDER_TARGET_PROPERTIES oHwndRenderTargetProperties = D2D1::HwndRenderTargetProperties( hWnd, oPixelSize );
		
			// ID2D1HwndRenderTargetの生成
		    hResult = pD2d1Factory->CreateHwndRenderTarget(
					oRenderTargetProperties,
					oHwndRenderTargetProperties,
					&pRenderTarget);
		    if ( FAILED( hResult ) )
			{	std::wcout << L"CreateHwndRenderTarget失敗" << std::endl;
				return false;	// default
		    }
		}
		
		// ウインドウを表示する
		::ShowWindow( hWnd, SW_SHOW );
		return true;
	}
	
	public:
	bool	WindowProcDelete(HWND hWnd, WPARAM wParam, LPARAM lParam)
	{				
		// ID2D1HwndRenderTargetの破棄
		if ( NULL != pRenderTarget )	pRenderTarget->Release();
		// ID2D1Factoryの破棄
	    if ( NULL != pD2d1Factory )		pD2d1Factory->Release();
		// |WICImagingFactoryの破棄
		if ( NULL != pWICImagingFactory )	pWICImagingFactory->Release();
	 
		// 終了する（ 引数はそのまま終了コードとなります ）
		::PostQuitMessage( 0 );
		return true;
	}

	public:
	bool	WindowProcSize(HWND hWnd, WPARAM wParam, LPARAM lParam)
	{	D2D1_SIZE_U oPixelSize = { LOWORD( lParam ), HIWORD( lParam ) };
		pRenderTarget->Resize( &oPixelSize );
		return true;
	}

	public:
	bool	WindowProcPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
	{	HRESULT hResult = S_OK;

		// ターゲットサイズの取得
	    D2D1_SIZE_F oTargetSize = pRenderTarget->GetSize();
	 
		// 描画開始
		PAINTSTRUCT tPaintStruct;
		::BeginPaint( hWnd, &tPaintStruct );
	 
		// 描画開始(Direct2D)
		pRenderTarget->BeginDraw();
	 
		// 背景のクリア
		D2D1_COLOR_F oBKColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		pRenderTarget->Clear( oBKColor );
	 
		// テキストの描画
		IWICBitmapDecoder* pWICBitmapDecoder = NULL;
		IWICBitmapFrameDecode* pWICBitmapFrame = NULL;
		IWICFormatConverter* pFormatConverter = NULL;
		ID2D1Bitmap* pBitmap = NULL;
	 
		// デコーダ生成( File to Bitmap )
		if(SUCCEEDED(hResult))
		{	hResult = pWICImagingFactory->CreateDecoderFromFilename(
				L"e513e8c8.jpg",
	            NULL,
				GENERIC_READ,
				WICDecodeMetadataCacheOnLoad,
				&pWICBitmapDecoder);
			if(FAILED(hResult))		std::wcout << L"CreateDecoderFromFilename失敗" << std::endl;
		}
	 
		// ビットマップのフレーム取得
		if(SUCCEEDED(hResult))
		{	hResult = pWICBitmapDecoder->GetFrame( 0, &pWICBitmapFrame );
	        if(FAILED(hResult))		std::wcout << L"GetFrame失敗" << std::endl;
		}

		// フォーマットコンバータ生成
	    if(SUCCEEDED(hResult))
		{	hResult = pWICImagingFactory->CreateFormatConverter( &pFormatConverter );
			if(FAILED(hResult))		std::wcout << L"CreateFormatConverter失敗" << std::endl;
		}
	 
		// フォーマットコンバータ初期化
	    if(SUCCEEDED(hResult))
		{	hResult = pFormatConverter->Initialize(
				pWICBitmapFrame,					// BitmapSource
	            GUID_WICPixelFormat32bppPBGRA,		// ピクセルフォーマット
	            WICBitmapDitherTypeNone,			// BitmapDitherType
	            NULL,								// バレット
	            1.0f,								// 透過率
	            WICBitmapPaletteTypeMedianCut);		// パレットタイプ
			if(FAILED(hResult))		std::wcout << L"Initialize失敗" << std::endl;
		}	 
	 
		// BitmapSource -> Bitmapへ変換
		if(SUCCEEDED(hResult))
		{	hResult = pRenderTarget->CreateBitmapFromWicBitmap( pFormatConverter, NULL, &pBitmap );
			if(FAILED(hResult))		std::wcout << L"CreateBitmapFromWicBitmap失敗" << std::endl;
		}	 
	 
		// Bitmapの描画
		if(SUCCEEDED(hResult))
		{

#if 1
			// ビットマップのサイズを取得
			D2D1_SIZE_F tBitmapSize = pBitmap->GetSize(); 
			D2D_POINT_2F tLeftTop = D2D1::Point2F(
				( oTargetSize.width  - tBitmapSize.width  ) / 2,
				( oTargetSize.height - tBitmapSize.height ) / 2);
	 	
			// 描画矩形(画面中央)
			D2D1_RECT_F oDrawRect = D2D1::RectF(
				tLeftTop.x,							// left
				tLeftTop.y,							// top
				tLeftTop.x + tBitmapSize.width,		// right
				tLeftTop.y + tBitmapSize.height);	// bottom
			
			// Bitmapの描画
	        pRenderTarget->DrawBitmap( pBitmap, oDrawRect );
#else		

			pRenderTarget->BeginDraw();
			pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::LightYellow));

			D2D1_SIZE_F size = pRenderTarget->GetSize();
			float rx = size.width / 2;
			float ry = size.height / 2;
    
			D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(rx, ry), rx, ry);
    
			ID2D1SolidColorBrush* brush;
			pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange), &brush);
			pRenderTarget->FillEllipse(ellipse, brush);
			brush->Release();

			pRenderTarget->EndDraw();
#endif
		}

		// Bitmap の保存
		SaveToFile(L"R:\\output.png");

		// IBitmapの破棄
		if( NULL != pBitmap )	pBitmap->Release();
		// IFormatConverterの破棄
	    if ( NULL != pFormatConverter )		pFormatConverter->Release();
		// IWICBitmapFrameの破棄
		if( NULL != pWICBitmapFrame )		pWICBitmapFrame->Release();
		// IWICBitmapDecoderの破棄
		if( NULL != pWICBitmapDecoder )		pWICBitmapDecoder->Release();
	 
		// 描画終了(Direct2D)
		pRenderTarget->EndDraw();
	 
		// 描画終了
		::EndPaint( hWnd, &tPaintStruct );
		
		return true;
	}
	
	/// <summary>
	/// ファイル保存
	/// 試験用
	/// </summary>
	public:
	void SaveToFile(WCHAR* filename)
	{
		HRESULT hr = S_OK;
		
		IWICStream*				pStream			= NULL;
		IWICBitmapEncoder*		pEncoder		= NULL;
		IWICBitmapFrameEncode*	pFrameEncode	= NULL;
		
	    D2D1_SIZE_F oTargetSize = pRenderTarget->GetSize();
		D2D1_RECT_U Rect = D2D1::RectU(0, 0, pRenderTarget->GetSize().width, pRenderTarget->GetSize().height);
		D2D1_BITMAP_PROPERTIES	bmpp;
		ID2D1Bitmap* bitmap = NULL;
		bmpp.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;	// DXGI_FORMAT_UNKNOWN;
		bmpp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
		
		hr	= pRenderTarget->CreateBitmap(D2D1::SizeU((UINT32)oTargetSize.width, (UINT32)oTargetSize.height), bmpp, &bitmap);
		if(!SUCCEEDED(hr))	return;

		D2D1_POINT_2U destPoint = D2D1::Point2U();
		bitmap->CopyFromRenderTarget(&destPoint, pRenderTarget, &Rect);

//		ID2D1Bitmap::CopyFromRenderTarget(

//		pRenderTarget->

		if (SUCCEEDED(hr))
		{
		    hr = pWICImagingFactory->CreateStream(&pStream);
		}
		
		WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
		if (SUCCEEDED(hr))
		{
		    hr = pStream->InitializeFromFilename(filename, GENERIC_WRITE);
		}
		if (SUCCEEDED(hr))
		{
		    hr = pWICImagingFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder);
		}
		if (SUCCEEDED(hr))
		{
		    hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
		}
		if (SUCCEEDED(hr))
		{
		    hr = pEncoder->CreateNewFrame(&pFrameEncode, NULL);
		}
		if (SUCCEEDED(hr))
		{
		    hr = pFrameEncode->Initialize(NULL);
		}
		if (SUCCEEDED(hr))
		{//	UINT32	ww,	hh;
		//	bitmap->GetSize(&ww, &hh);
		    hr = pFrameEncode->SetSize((UINT)oTargetSize.width, (UINT)oTargetSize.height);
		}
		if (SUCCEEDED(hr))
		{
		    hr = pFrameEncode->SetPixelFormat(&format);
		}

//		IWICImageEncoder* imageEncoder;
//		if (SUCCEEDED(hr))
//		{	hr	= pWICFactory->CreateImageEncoder(d2dDevice.Get(), &imageEncoder)
//		}

		if (SUCCEEDED(hr))
		{	//    hr = pFrameEncode->WriteSource(&bitmap, NULL);
		    hr = pFrameEncode->WriteSource((IWICBitmap*)&bitmap, NULL);
		
		}
		if (SUCCEEDED(hr))
		{
		    hr = pFrameEncode->Commit();
		}
		if (SUCCEEDED(hr))
		{
		    hr = pEncoder->Commit();
		}

		SAFE_RELEASE(pEncoder);
		SAFE_RELEASE(pFrameEncode);
		SAFE_RELEASE(pStream);
	}

	public:
	int		ClassMain()
	{	
		WNDCLASSEX	tWndClass;
		HINSTANCE	hInstance;
		TCHAR*		cpClassName;
		TCHAR*		cpWindowName;
		TCHAR*		cpMenu;
		HWND		hWnd;
		MSG			tMsg;
	
	
		// アプリケーションインスタンス
		hInstance	= ::GetModuleHandle( NULL );
	
		// クラス名称
		cpClassName	= _T("MainWindowClass");
	
		// メニュー
		cpMenu		= MAKEINTRESOURCE( NULL );
	
		// ウインドウ名称
		cpWindowName = _T("Direct2Dでbmpを描画");
	
		// ウインドウクラスパラメータセット
		tWndClass.cbSize		= sizeof( WNDCLASSEX );
		tWndClass.style			= CS_HREDRAW | CS_VREDRAW;
		tWndClass.lpfnWndProc	= &eMainWindowProc;
		tWndClass.cbClsExtra	= 0;	// ::GetClassLong で取得可能なメモリ
		tWndClass.cbWndExtra	= 0;	// ::GetWindowLong で取得可能なメモリ
		tWndClass.hInstance		= hInstance; 
		tWndClass.hIcon			= ::LoadIcon( NULL, IDI_APPLICATION );
		tWndClass.hCursor		= ::LoadCursor( NULL, IDC_ARROW );
		tWndClass.hbrBackground	= (HBRUSH)( COLOR_WINDOW + 1 );
		tWndClass.lpszMenuName	= cpMenu;
		tWndClass.lpszClassName	= cpClassName;
		tWndClass.hIconSm		= NULL;
	
		// ウインドウクラス生成
		if(0==::RegisterClassEx(&tWndClass))
		{	/* 失敗 */
			return( -1 );
		}
	
	    // ウインドウを生成する
	    hWnd = ::CreateWindowEx (
	          0                       // extended window style
	        , tWndClass.lpszClassName // pointer to registered class name
	        , cpWindowName            // pointer to window name
	        , WS_OVERLAPPEDWINDOW     // window style
	        , CW_USEDEFAULT           // horizontal position of window
	        , CW_USEDEFAULT           // vertical position of window
	        , 640                     // window width
	        , 480                     // window height
	        , NULL                    // handle to parent or owner window
	        , NULL                    // handle to menu, or child-window identifier
	        , hInstance               // handle to application instance
	        , (VOID*)0x12345678       // pointer to window-creation data
	    );
	 
	 
	    /*
	        メッセージループ
	    */
	    while( 0 != ::GetMessage( &tMsg, NULL, 0, 0 ) ) {
	        ::TranslateMessage ( &tMsg );
	        ::DispatchMessage ( &tMsg );
	    }
 
	    // WM_QUITの終了コードを返却する
		return( tMsg.wParam );
	}
};



class D2DtestClass* pMainClass;


/*
    メインウインドウイベント処理
*/
LRESULT CALLBACK eMainWindowProc(
	HWND   hWnd,  // handle to window
    UINT   uMsg,   // message identifier
    WPARAM wParam, // first message parameter
    LPARAM lParam // second message parameter
)
{	switch( uMsg )
	{	case WM_CREATE:
			pMainClass->WindowProcCreate(hWnd, wParam, lParam);
	        break;
 
		case WM_DESTROY:
			pMainClass->WindowProcDelete(hWnd, wParam, lParam);
			break;
 
		case WM_SIZE:
			pMainClass->WindowProcSize(hWnd, wParam, lParam);
			break;

		case WM_ERASEBKGND:
			return( TRUE );
 
		case WM_PAINT:
			pMainClass->WindowProcPaint(hWnd, wParam, lParam);
			return( FALSE );
	}
 
	// デフォルト処理呼び出し
	return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
}





/*
    Direct2Dでbmpを描画する
*/
int _tmain(int argc, _TCHAR* argv[])
{
	//	std::wcoutのロケールを設定
	std::wcout.imbue( std::locale( "", std::locale::ctype ) );

	// COM初期化
	::CoInitialize( NULL );

	pMainClass = new D2DtestClass();
	pMainClass->ClassMain();
	
	// COM初期化
    ::CoUninitialize();
 
    // WM_QUITの終了コードを返却する
	// return( tMsg.wParam );
	return 0;
}
 
 