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

template<class Interface>
inline void SafeRelease(Interface **ppInterfaceToRelease)
{	if(*ppInterfaceToRelease != NULL)
	{	(*ppInterfaceToRelease)->Release();
		(*ppInterfaceToRelease) = NULL;
	}
}
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if(p)      { (p)->Release(); (p)=NULL; } }
#endif

class D2DtestClass
{
	// ID2D1
	private:	ID2D1Factory*			pD2DFactory			= NULL;
	private:	ID2D1RenderTarget*		pD2DRenderTarget	= NULL;

	// IWIC
	private:	IWICImagingFactory*		pWICFactory			= NULL;
    private:	IWICBitmap*				pWICBitmap			= NULL;

	public:
	~D2DtestClass( )
	{	SafeRelease(&pD2DRenderTarget);
		SafeRelease(&pWICBitmap);
		SafeRelease(&pWICFactory);
		SafeRelease(&pD2DFactory);
	}

	public:
	bool	Create( )
	{	HRESULT hr	= S_OK;
		
		if(SUCCEEDED(hr) && pD2DFactory==NULL)
		{	hr	= D2D1CreateFactory(
			//	D2D1_FACTORY_TYPE_SINGLE_THREADED,
				D2D1_FACTORY_TYPE_MULTI_THREADED,
				&pD2DFactory);
		}

		if(SUCCEEDED(hr) && pWICFactory==NULL)
		{	hr	= CoCreateInstance(
				CLSID_WICImagingFactory,
				NULL,
				CLSCTX_INPROC_SERVER,
				IID_IWICImagingFactory,
				reinterpret_cast<void **> (&pWICFactory));
		}

		if(SUCCEEDED(hr) && pWICBitmap==NULL)
		{	hr = pWICFactory->CreateBitmap(
				1920,
				1080,
				GUID_WICPixelFormat64bppPRGBA,	// GUID_WICPixelFormat32bppPRGBA,
				WICBitmapCacheOnLoad,
				&pWICBitmap);
		}

		// RenderTarget の設定
		if(SUCCEEDED(hr) && pD2DRenderTarget==NULL)
		{	// CreateDCRenderTarget				Windows グラフィックスデバイスインターフェイス (GDI) デバイスコンテキストに描画するレンダーターゲットを作成
			// CreateDxgiSurfaceRenderTarget	DirectX Graphics Infrastructure (DXGI) サーフェイスに描画するレンダーターゲットを作成
			// CreateHwndRenderTarget			ウィンドウにレンダリングするレンダーターゲットである ID2D1HwndRenderTarget を作成
			// CreateWicBitmapRenderTarget		Microsoft Windows Imaging Component (WIC) ビットマップにレンダリングするレンダーターゲットを作成
		
			D2D1_RENDER_TARGET_PROPERTIES	props	= D2D1::RenderTargetProperties();

			D2D1_RENDER_TARGET_TYPE	types[ ]	=
			{	D2D1_RENDER_TARGET_TYPE_HARDWARE,
				D2D1_RENDER_TARGET_TYPE_DEFAULT,
				D2D1_RENDER_TARGET_TYPE_SOFTWARE
			};

			DXGI_FORMAT		digif[ ]	=
			{	DXGI_FORMAT_R32G32B32A32_TYPELESS,
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				DXGI_FORMAT_R32G32B32A32_UINT,
				DXGI_FORMAT_R32G32B32A32_SINT,
				DXGI_FORMAT_R16G16B16A16_TYPELESS,
				DXGI_FORMAT_R16G16B16A16_FLOAT,
				DXGI_FORMAT_R16G16B16A16_UNORM,
				DXGI_FORMAT_R16G16B16A16_UINT,
				DXGI_FORMAT_R16G16B16A16_SNORM,
				DXGI_FORMAT_R16G16B16A16_SINT
			};

			D2D1_ALPHA_MODE	alpha[ ]	=
			{	D2D1_ALPHA_MODE_UNKNOWN,
				D2D1_ALPHA_MODE_PREMULTIPLIED,
				D2D1_ALPHA_MODE_STRAIGHT,
				D2D1_ALPHA_MODE_IGNORE
			};

			for(int tt=0; tt<sizeof(types)/sizeof(D2D1_RENDER_TARGET_TYPE); tt++)
			{	props.type	= types[tt];

				for(int dd=0; dd<sizeof(digif)/sizeof(DXGI_FORMAT); dd++)
				{	for(int aa=0; aa<=sizeof(alpha)/sizeof(D2D1_ALPHA_MODE); aa++)
					{	props.pixelFormat	= D2D1::PixelFormat(
							digif[dd],		// DXGI_FORMAT_R16G16B16A16_UNORM,	// DXGI_FORMAT_B8G8R8A8_UNORM,
							alpha[aa]);		// D2D1_ALPHA_MODE_IGNORE);

						hr	= pD2DFactory->CreateWicBitmapRenderTarget(
							pWICBitmap,
							props,
							&pD2DRenderTarget);
						if(SUCCEEDED(hr))
						{	printf("ok - type:%02d, format:%02d, alpha:%02d\n", tt, dd, aa);
							break;
						}
						else
						{	printf("ng - type:%02d, format:%02d, alpha:%02d\n", tt, dd, aa);						
	
						}
					}
					if(SUCCEEDED(hr))	break;
				}
				if(SUCCEEDED(hr))	break;
			}
		}
		return SUCCEEDED(hr) ? true : false;
	}
	

	public:
	bool	Paint( )
	{	HRESULT hr = S_OK;

		// 描画開始(Direct2D)
		pD2DRenderTarget->BeginDraw();
	 
		// 背景のクリア
		D2D1_COLOR_F oBKColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		pD2DRenderTarget->Clear( oBKColor );
	 
		// Bitmapの描画
		if(SUCCEEDED(hr))
		{	// ターゲットサイズの取得
			D2D1_SIZE_F oTargetSize = pD2DRenderTarget->GetSize();
			D2D1_RECT_F oTargetRect ={ 0, 0, oTargetSize.width, oTargetSize.height };

			// float rx = oTargetSize.width / 2;
			// float ry = oTargetSize.height / 2;
    		// 
			// D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(rx, ry), rx, ry);
    		// 
			// ID2D1SolidColorBrush* brush;
			// pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange), &brush);
			// pD2DRenderTarget->FillEllipse(ellipse, brush);
			// brush->Release();


			ID2D1LinearGradientBrush *m_pLinearGradientBrush;
			// Create an array of gradient stops to put in the gradient stop
			// collection that will be used in the gradient brush.
			ID2D1GradientStopCollection *pGradientStops = NULL;

			D2D1_GRADIENT_STOP gradientStops[2];
			gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::Yellow, 1);
			gradientStops[0].position = 0.0f;
			gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::ForestGreen, 1);
			gradientStops[1].position = 1.0f;
			// Create the ID2D1GradientStopCollection from a previously
			// declared array of D2D1_GRADIENT_STOP structs.
			hr = pD2DRenderTarget->CreateGradientStopCollection(
			    gradientStops,
			    2,
			    D2D1_GAMMA_2_2,
			    D2D1_EXTEND_MODE_CLAMP,
			    &pGradientStops);

			hr = pD2DRenderTarget->CreateLinearGradientBrush(
				D2D1::LinearGradientBrushProperties(
				D2D1::Point2F(0, 0),
				D2D1::Point2F(oTargetSize.width, oTargetSize.height)),
				pGradientStops,
				&m_pLinearGradientBrush);

			pD2DRenderTarget->FillRectangle(&oTargetRect, m_pLinearGradientBrush);
		//	pD2DRenderTarget->DrawRectangle(&rcBrushRect, m_pBlackBrush, 1, NULL);
		}

		// 描画終了(Direct2D)
		pD2DRenderTarget->EndDraw();
		
		return SUCCEEDED(hr) ? true : false;
	}
	
	/// <summary>
	/// ファイル保存
	/// 試験用
	/// </summary>
	public:
	void SaveToFile(WCHAR* filename)
	{	HRESULT hr = S_OK;
		
		IWICStream*				pStream			= NULL;
		IWICBitmapEncoder*		pEncoder		= NULL;
		IWICBitmapFrameEncode*	pFrameEncode	= NULL;

		if (SUCCEEDED(hr))
		{
		    hr = pWICFactory->CreateStream(&pStream);
		}
		
		WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
		if (SUCCEEDED(hr))
		{
		    hr = pStream->InitializeFromFilename(filename, GENERIC_WRITE);
		}
		if (SUCCEEDED(hr))
		{
		    hr = pWICFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder);
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
		{	UINT32	ww,	hh;
			pWICBitmap->GetSize(&ww, &hh);
		    hr = pFrameEncode->SetSize(ww, hh);
		}
		if (SUCCEEDED(hr))
		{
		    hr = pFrameEncode->SetPixelFormat(&format);
		}
		if (SUCCEEDED(hr))
		{
		    hr = pFrameEncode->WriteSource(pWICBitmap, NULL);
		}
		if (SUCCEEDED(hr))
		{
		    hr = pFrameEncode->Commit();
		}
		if (SUCCEEDED(hr))
		{
		    hr = pEncoder->Commit();
		}

		SafeRelease(&pEncoder);
		SafeRelease(&pFrameEncode);
		SafeRelease(&pStream);
	}
};



/*
    Direct2Dでbmpを描画する
*/
int _tmain(int argc, _TCHAR* argv[])
{	::CoInitialize( NULL );

	D2DtestClass*	pMainClass = new D2DtestClass();
	pMainClass->Create( );

	pMainClass->Paint( );
		
	// Bitmap の保存
	pMainClass->SaveToFile(L"R:\\output.png");
	
	delete pMainClass;
	
    ::CoUninitialize();
	return 0;
}
 
 