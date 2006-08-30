#pragma once
#include "drkPvr.h"

namespace TASplitter
{
	//void TaWrite(u32* data);
	void TADma(u32 address,u32* data,u32 size);

	bool Ta_Init();
	void Ta_Term();
	void Ta_Reset(bool manual);

	//hehe
	//as it sems , bit 1,2 are type , bit 0 is mod volume :p
	const u32 ListType_Opaque=0;			
	const u32 ListType_Opaque_Modifier_Volume=1;
	const u32 ListType_Translucent	=2;
	const u32 ListType_Translucent_Modifier_Volume=3;
	const u32 ListType_Punch_Through=4;
#define IsModVolList(list) (((list)&1)!=0)

	//misc ones
	const u32 ListType_None=-1;

	//Control Parameter
	const u32 ParamType_End_Of_List=0;
	const u32 ParamType_User_Tile_Clip=1;
	const u32 ParamType_Object_List_Set=2;

	//Global Parameter
	const u32 ParamType_Polygon_or_Modifier_Volume=4;
	const u32 ParamType_Sprite=5;

	//Vertex , Sprite or ModVolume Parameter
	const u32 ParamType_Vertex_Parameter=7;

	//Reserved
	const u32 ParamType_Reserved_1=3;
	const u32 ParamType_Reserved_2=6;

	const u32 SZ32=1;
	const u32 SZ64=2;

#pragma pack(push, 1)   // n = 1
	//	Global Param/misc structs
	//4B
	struct PCW
	{
		//Obj Control
		u32 UV_16bit		: 1;
		u32 Gouraud			: 1;
		u32 Offset			: 1;
		u32 Texture			: 1;
		u32 Col_Type		: 2;
		u32 Volume			: 1;
		u32 Shadow			: 1;
		u32	Reserved		: 8;

		// Group Control
		u32 User_Clip		: 2;
		u32 Strip_Len		: 2;
		u32 Res_2			: 3;
		u32 Group_En		: 1;

		// Para Control
		u32 ListType		: 3;
		u32 Res_1			: 1;
		u32 EndOfStrip		: 1;
		u32	ParaType		: 3;		
	} ;


	//// ISP/TSP Instruction Word

	struct ISP_TSP
	{
		u32	Reserved	: 20;
		u32	DCalcCtrl	: 1;
		u32	CacheBypass	: 1;
		u32	UV_16b		: 1;	// redundant in many places
		u32	Gouraud		: 1;
		u32	Offset		: 1;
		u32	Texture		: 1;
		u32	ZWriteDis	: 1;
		u32	CullMode	: 2;
		u32	DepthMode	: 3;

	};


	//// END ISP/TSP Instruction Word


	//// TSP Instruction Word

	union TSP
	{
		struct 
		{
			u32 TexV		: 3;
			u32 TexU		: 3;
			u32 ShadInstr	: 2;
			u32 MipMapD		: 4;
			u32 SupSample	: 1;
			u32 FilterMode	: 2;
			u32 ClampUV		: 2;
			u32 FlipUV		: 2;
			u32 IgnoreTexA	: 1;
			u32 UseAlpha	: 1;
			u32 ColorClamp	: 1;
			u32 FogCtrl		: 2;
			u32 DstSelect	: 1;	// Secondary Accum
			u32 SrcSelect	: 1;	// Primary Accum
			u32 DstInstr	: 3;
			u32 SrcInstr	: 3;
		};
		u32 full;
	} ;


	//// END TSP Instruction Word


	/// Texture Control Word
	union TCW
	{
		struct
		{
			u32	TexAddr		:21;
			u32	Reserved	: 4;
			u32 StrideSel	: 1;
			u32 ScanOrder	: 1;
			u32	PixelFmt	: 3;
			u32 VQ_Comp		: 1;
			u32	MipMapped	: 1;
		} NO_PAL;
		struct
		{
			u32 TexAddr		:21;
			u32 PalSelect	: 6;
			u32 PixelFmt	: 3;
			u32 VQ_Comp		: 1;
			u32 MipMapped	: 1;
		} PAL;
		u32 full;
	};

	/// END Texture Control Word

	//32B
	struct Ta_Dma
	{
		//0
		//Parameter Control Word
		PCW pcw;
		//4
		union
		{
			u8  data_8[32-4];
			u32 data_32[8-1];
		};
	};

	//32B
	struct TA_PolyParamA
	{
		PCW pcw;
		ISP_TSP isp;

		//for 1st volume/normal
		TSP tsp;
		TCW tcw;

		//if 2 volume format/ingored
		TSP tsp1;
		TCW tcw1;

		//for sort dma
		u32 SDMA_SIZE;
		u32 SDMA_ADDR;
	};
	//32B
	struct TA_PolyParamB
	{
		//Face color
		f32 Col0A, Col0R,Col0G, Col0B;
		//Face color 1 / Offset color :)
		f32 Col1A, Col1R, Col1G, Col1B;
	};
	struct TA_ModVolParam
	{
		Ta_Dma nil;
	};
	struct TA_SpriteParam
	{
		Ta_Dma nil;
	};

	//	Vertex Param Structs
	//28B
	struct TA_Vertex0	//	(Non-Textured, Packed Color)
	{
		f32 xyz[3];
		u32 ignore_1,
			ignore_2;
		u32 BaseCol;
		u32 ignore_3;
	};
	//28B
	struct TA_Vertex1	//	(Non-Textured, Floating Color)
	{
		f32 xyz[3];
		f32 BaseA, BaseR,
			BaseG, BaseB;
	};
	//28B
	struct TA_Vertex2	//	(Non-Textured, Intensity)
	{
		f32 xyz[3];
		u32 ignore_1,
			ignore_2;
		f32 BaseInt;
		u32 ignore_3;
	};
	//28B
	struct TA_Vertex3	//	(Packed Color)
	{
		f32 xyz[3];
		f32 u,v;
		u32 BaseCol;
		u32 OffsCol;
	};
	//28B
	struct TA_Vertex4	//	(Packed Color, 16bit UV)
	{
		f32 xyz[3];
		u32 u : 16;
		u32 v : 16;
		u32 ignore_1;
		u32 BaseCol;
		u32 OffsCol;
	};
	//28B
	struct TA_Vertex5A	//	(Floating Color)	
	{
		f32 xyz[3];
		f32 u,v;
		u32 ignore_1;
		u32 ignore_2;
	};
	//32B
	struct TA_Vertex5B
	{
		f32 BaseA, BaseR,
			BaseG, BaseB;
		f32 OffsA, OffsR,
			OffsG, OffsB;
	};
	//28B
	struct TA_Vertex6A	//	(Floating Color, 16bit UV)
	{
		f32 xyz[3];
		u32 u : 16;
		u32 v : 16;
		u32 ignore_1;
		u32 ignore_2;
		u32 ignore_3;
	};
	//32B
	struct TA_Vertex6B
	{
		f32 BaseA, BaseR,
			BaseG, BaseB;
		f32 OffsA, Offs,
			OffsG, OffsB;
	};
	//28B
	struct TA_Vertex7	//	(Intensity)
	{
		f32 xyz[3];
		f32 u,v;
		f32 BaseInt;
		f32 OffsInt;

	};
	//28B
	struct TA_Vertex8	//	(Intensity, 16bit UV)
	{
		f32 xyz[3];
		u32 u : 16;
		u32 v : 16;
		u32 ignore_1;
		f32 BaseInt;
		f32 OffsInt;
	};
	//28B
	struct TA_Vertex9	//	(Non-Textured, Packed Color, with Two Volumes)
	{
		f32 xyz[3];
		u32 BaseCol0;
		u32 BaseCol1;
		u32 ignore_1;
		u32 ignore_2;
	};
	//28B
	struct TA_Vertex10	//	(Non-Textured, Intensity, with Two Volumes)
	{
		f32 xyz[3];
		f32 BaseInt0;
		f32 BaseInt1;
		u32 ignore_1;
		u32 ignore_2;
	};

	//28B
	struct TA_Vertex11A	//	(Textured, Packed Color, with Two Volumes)
	{
		f32 xyz[3];
		f32 u0,v0;
		u32 BaseCol0, OffsCol0;
	};
	//32B
	struct TA_Vertex11B
	{
		f32 u1,v1;
		u32 BaseCol1, OffsCol1;
		u32 ignore_1, ignore_2;
		u32 ignore_3, ignore_4;
	};

	//28B
	struct TA_Vertex12A	//	(Textured, Packed Color, 16bit UV, with Two Volumes)
	{
		f32 xyz[3];
		u32 u0 : 16;
		u32 v0 : 16;
		u32 ignore_1;
		u32 BaseCol0, OffsCol0;
	};
	//32B
	struct TA_Vertex12B
	{
		u32 u1 : 16;
		u32 v1 : 16;
		u32 ignore_2;
		u32 BaseCol1, OffsCol1;
		u32 ignore_3, ignore_4;
		u32 ignore_5, ignore_6;
	};
	//28B
	struct TA_Vertex13A	//	(Textured, Intensity, with Two Volumes)
	{
		f32 xyz[3];
		f32 u0,v0;
		f32 BaseInt0, OffsInt0;
	};
	//32B
	struct TA_Vertex13B
	{
		f32 u1,v1;
		f32 BaseInt1, OffsInt1;
		u32 ignore_1, ignore_2;
		u32 ignore_3, ignore_4;
	};

	//28B
	struct TA_Vertex14A	//	(Textured, Intensity, 16bit UV, with Two Volumes)
	{
		f32 xyz[3];
		u32 u0 : 16;
		u32 v0 : 16;
		u32 ignore_1;
		f32 BaseInt0, OffsInt0;
	};
	//32B
	struct TA_Vertex14B
	{
		u32 u1 : 16;
		u32 v1 : 16;
		u32 ignore_2;
		f32 BaseInt1, OffsInt1;
		u32 ignore_3, ignore_4;
		u32 ignore_5, ignore_6;
	};

	//28B
	struct TA_Sprite0A	//	Line ?
	{
		f32 x0,y0,z0;
		f32 x1,y1,z1;
		f32 x2;

	};
	//32B
	struct TA_Sprite0B	//	Line ?
	{
		f32 y2,z2;
		f32 x3,y3;
		u32 ignore_1, ignore_2;
		u32 ignore_3, ignore_4;
	};
	//28B
	struct TA_Sprite1A
	{
		f32 x0,y0,z0;
		f32 x1,y1,z1;
		f32 x2;
	};
	//32B
	struct TA_Sprite1B
	{
		f32 y2,z2;
		f32 x3,y3;
		u32 ignore_1;

		u16 v0; u16 u0;	
		u16 v1; u16 u1;	
		u16 v2; u16 u2;	
	};

	//28B
	struct TA_ModVolA
	{
		f32 x0,y0,z0;
		f32 x1,y1,z1;
		f32 x2;	//3+3+1=7*4=28
	};
	//32B
	struct TA_ModVolB
	{
		f32 y2,z2;		//2
		u32 ignore[6];	//8
	};

	//all together , and pcw ;)
	struct TA_VertexParam
	{
		PCW pcw;

		union
		{
			u8 Raw[64-4];

			TA_Vertex0		vtx0;
			TA_Vertex1		vtx1;
			TA_Vertex2		vtx2;
			TA_Vertex3		vtx3;
			TA_Vertex4		vtx4;

			struct
			{
				TA_Vertex5A			vtx5A;
				TA_Vertex5B			vtx5B;
			};

			struct
			{
				TA_Vertex6A			vtx6A;
				TA_Vertex6B			vtx6B;
			};

			TA_Vertex7		vtx7;
			TA_Vertex8		vtx8;
			TA_Vertex9		vtx9;
			TA_Vertex10		vtx10;



			struct
			{
				TA_Vertex11A	vtx11A;
				TA_Vertex11B	vtx11B;
			};


			struct
			{
				TA_Vertex12A	vtx12A;
				TA_Vertex12B	vtx12B;
			};

			struct
			{
				TA_Vertex13A	vtx13A;
				TA_Vertex13B	vtx13B;
			};

			struct
			{
				TA_Vertex14A	vtx14A;
				TA_Vertex14B	vtx14B;
			};

			struct
			{
				TA_Sprite0A		spr0A;
				TA_Sprite0B		spr0B;
			};

			struct
			{
				TA_Sprite1A		spr1A;
				TA_Sprite1B		spr1B;
			};

			struct
			{
				TA_ModVolA		mvolA;
				TA_ModVolB		mvolB;
			};
		};

	};


#pragma pack(pop)

	typedef u32 fastcall TaListFP(Ta_Dma* data,u32 size);


	const InterruptID ListEndInterrupt[5]=
	{
		InterruptID::holly_OPAQUE,
		InterruptID::holly_OPAQUEMOD,
		InterruptID::holly_TRANS,
		InterruptID::holly_TRANSMOD,
		InterruptID::holly_PUNCHTHRU
	};
	
	//TA fifo state variables
	//Current List
	extern u32 CurrentList; 
	//Splitter function (normaly ta_dma_main , modified for split dma's)
	extern TaListFP* TaCmd;
	//Vertex Handler function :)
	extern TaListFP* VerxexDataFP;
	//finished lists
	extern bool ListIsFinished[5];

	//splitter function lookup
	extern TaListFP* ta_poly_data_lut[15];

	extern bool StripStarted;

	template<class TA_decoder>
	class FifoSplitter
	{
	public:

		//helper function for dummy dma's.Handles 32B and then switches to ta_main for next data
		static u32 fastcall ta_dummy_32(Ta_Dma* data,u32 size)
		{
			TaCmd=ta_main;
			return SZ32;
		}
		//Second part of poly data
		static u32 fastcall ta_poly_B_32(Ta_Dma* data,u32 size)
		{
			TA_decoder::AppendPolyParam64B((TA_PolyParamB*)data);
			TaCmd=ta_main;
			return SZ32;
		}

		//part : 0 fill all data , 1 fill upper 32B , 2 fill lower 32B
		//Poly decoder , will be moved to pvr code
		template <u32 poly_type,u32 part,bool StripEnd>
		__forceinline
		static u32 fastcall ta_handle_poly(Ta_Dma* data,u32 size)
		{
			TA_VertexParam* vp=(TA_VertexParam*)data;
			u32 rv=0;

			if (part==2)
				TaCmd=ta_main;

			switch (poly_type)
			{
#define ver_32B_def(num) \
	case num : {\
	TA_decoder::AppendPolyVertex##num(&vp->vtx##num);\
	rv=SZ32; }\
	break;

				//32b , allways in one pass :)
				ver_32B_def(0);//(Non-Textured, Packed Color)
				ver_32B_def(1);//(Non-Textured, Floating Color)
				ver_32B_def(2);//(Non-Textured, Intensity)
				ver_32B_def(3);//(Textured, Packed Color)
				ver_32B_def(4);//(Textured, Packed Color, 16bit UV)
				ver_32B_def(7);//(Textured, Intensity)
				ver_32B_def(8);//(Textured, Intensity, 16bit UV)
				ver_32B_def(9);//(Non-Textured, Packed Color, with Two Volumes)
				ver_32B_def(10);//(Non-Textured, Intensity,	with Two Volumes)

#undef ver_32B_def

#define ver_64B_def(num) \
	case num : {\
	/*process first half*/	\
		if (part!=2)	\
		{	\
		rv+=SZ32;	\
		TA_decoder::AppendPolyVertex##num##A(&vp->vtx##num##A);\
		}	\
		/*process second half*/	\
		if (part!=1)	\
		{	\
		TA_decoder::AppendPolyVertex##num##B(&vp->vtx##num##B);\
		rv+=SZ32;	\
		} }\
		break;


				//64b , may be on 2 pass
				ver_64B_def(5);//(Textured, Floating Color)
				ver_64B_def(6);//(Textured, Floating Color, 16bit UV)
				ver_64B_def(11);//(Textured, Packed Color,	with Two Volumes)	
				ver_64B_def(12);//(Textured, Packed Color, 16bit UV, with Two Volumes)
				ver_64B_def(13);//(Textured, Intensity,	with Two Volumes)
				ver_64B_def(14);//(Textured, Intensity, 16bit UV, with Two Volumes)
#undef ver_64B_def
			}
			if (StripEnd)
			{
				StripStarted=false;
				TA_decoder::EndPolyStrip();
			}
			return rv;
		};

		//Code Splitter/rooters
		static u32 fastcall ta_mod_vol_data(Ta_Dma* data,u32 size)
		{
			if (size==1)
			{
				//32B more needed , 32B done :)
				TaCmd=ta_dummy_32;
				return SZ32;
			}
			else
			{
				//all 64B done
				return SZ64;
			}
		}
		static u32 fastcall ta_sprite1B_data(Ta_Dma* data,u32 size)
		{
			//32B more needed , 32B done :)
			TaCmd=ta_main;
			TA_decoder::AppendSpriteVertex1N((TA_Sprite1B*)data);
			return SZ32;
		}
		static u32 fastcall ta_sprite1_data(Ta_Dma* data,u32 size)
		{
			if (size==1)
			{
				//32B more needed , 32B done :)
				TaCmd=ta_dummy_32;
				return SZ32;
			}
			else
			{
				TA_VertexParam* vp=(TA_VertexParam*)data;
				TA_decoder::AppendSpriteVertex1A(&vp->spr1A);
				TA_decoder::AppendSpriteVertex1B(&vp->spr1B);
				//all 64B done
				return SZ64;
			}
		}


		template <u32 poly_type,u32 poly_size>
		static u32 fastcall ta_poly_data(Ta_Dma* dt,u32 size)
		{
			u32 ci=0;
			Ta_Dma* cdp=dt;
			if (StripStarted==false)
			{
				TA_decoder::StartPolyStrip();
				StripStarted=true;
			}

			while(size>=poly_size) 
			{
				verify(cdp->pcw.ParaType==ParamType_Vertex_Parameter);

				ci+=poly_size;
				size-=poly_size;
				ta_handle_poly<poly_type,0,false>(cdp,0);
		
				if (cdp->pcw.EndOfStrip)
					goto strip_end;
				
				cdp+=poly_size;
			}

			
			if ((poly_size!=SZ32) && (size==SZ32))//32B part of 64B
			{
				ta_handle_poly<poly_type,1,false>(cdp,0);
				if (cdp->pcw.EndOfStrip)
					TaCmd=ta_handle_poly<poly_type,2,true>;//end strip after part B is  done :)
				else
					TaCmd=ta_handle_poly<poly_type,2,false>;
				ci+=SZ32;
				size-=SZ32;//0'd
			}
			
			return ci;

strip_end:
			StripStarted=false;
			TA_decoder::EndPolyStrip();
			return ci;
		}

		//helpers
		static u32 fastcall poly_data_type_id(Ta_Dma* data)
		{
			if (data->pcw.Texture)
			{
				//textured
				if (data->pcw.Volume==0)
				{	//single volume
					if (data->pcw.Col_Type==0)
					{
						if (data->pcw.UV_16bit==0)
							return 3;					//(Textured, Packed Color , 32b uv)
						else
							return 4;					//(Textured, Packed Color , 16b uv)
					}
					else if (data->pcw.Col_Type==1)
					{
						if (data->pcw.UV_16bit==0)
							return 5;					//(Textured, Floating Color , 32b uv)
						else
							return 6;					//(Textured, Floating Color , 16b uv)
					}
					else
					{
						if (data->pcw.UV_16bit==0)
							return 7;					//(Textured, Intensity , 32b uv)
						else
							return 8;					//(Textured, Intensity , 16b uv)
					}
				}
				else
				{
					//two volumes
					if (data->pcw.Col_Type==0)
					{
						if (data->pcw.UV_16bit==0)
							return 11;					//(Textured, Packed Color, with Two Volumes)	

						else
							return 12;					//(Textured, Packed Color, 16bit UV, with Two Volumes)

					}
					else if (data->pcw.Col_Type==1)
					{
						log ("invalid");
					}
					else
					{
						if (data->pcw.UV_16bit==0)
							return 13;					//(Textured, Intensity, with Two Volumes)	

						else
							return 14;					//(Textured, Intensity, 16bit UV, with Two Volumes)
					}
				}
			}
			else
			{
				//non textured
				if (data->pcw.Volume==0)
				{	//single volume
					if (data->pcw.Col_Type==0)
						return 0;						//(Non-Textured, Packed Color)
					else if (data->pcw.Col_Type==1)
						return 1;						//(Non-Textured, Floating Color)
					else
						return 2;						//(Non-Textured, Intensity)
				}
				else
				{
					//two volumes
					if (data->pcw.Col_Type==0)
						return 9;						//(Non-Textured, Packed Color, with Two Volumes)
					else if (data->pcw.Col_Type==1)
					{
						log ("invalid");
					}
					else
						return 10;						//(Non-Textured, Intensity, with Two Volumes)
				}
			}
			return 0;
		}
		static u32 fastcall poly_header_type_size(Ta_Dma* data)
		{
			if (data->pcw.Col_Type>1)
			{
				if ((data->pcw.Offset==1) || (data->pcw.Volume==1))
					return SZ64;
			}
			return SZ32;
		}


		static void fastcall ta_list_start(u32 new_list)
		{
			verify(ListIsFinished[new_list]==false);
			CurrentList=new_list;
			TA_decoder::StartList(CurrentList);
		}

		static u32 fastcall ta_main(Ta_Dma* data,u32 size)
		{
			u32 ci=size;
			do
			{
				switch (data->pcw.ParaType)
				{
					//Control parameter
					//32B
				case ParamType_End_Of_List:
					{
						if (CurrentList==ListType_None)
							CurrentList=data->pcw.ListType;
						else
							TA_decoder::EndList(CurrentList);//end a list olny if it was realy started

						RaiseInterrupt(ListEndInterrupt[CurrentList]);
						ListIsFinished[CurrentList]=true;
						CurrentList=ListType_None;
						VerxexDataFP=0;
						data+=SZ32;
						size-=SZ32;
					}
					break;
					//32B
				case ParamType_User_Tile_Clip:
					{
						//*couh* ignore it :p
						data+=SZ32;
						size-=SZ32;
					}
					break;	
					//32B
				case ParamType_Object_List_Set:
					{
						//*couh* ignore it :p
						data+=SZ32;
						size-=SZ32;
					}
					break;

					//Global Parameter
					//ModVolue :32B
					//PolyType :32B/64B
				case ParamType_Polygon_or_Modifier_Volume:
					{

						if (CurrentList==ListType_None)
							ta_list_start(data->pcw.ListType);	//start a list ;)

						if (IsModVolList(CurrentList))
						{	//accept mod data
							VerxexDataFP=ta_mod_vol_data;
							data+=SZ32;
							size-=SZ32;
						}
						else
						{
							u32 id=poly_data_type_id(data);
							u32 psz=poly_header_type_size(data);

							VerxexDataFP=ta_poly_data_lut[id];
							verify(StripStarted==false);
							
							if (psz>size)
							{
								TA_decoder::AppendPolyParam64A((TA_PolyParamA*)data);
								//Handle next 32B ;)
								TaCmd=ta_poly_B_32;
								data+=SZ32;
								size-=SZ32;
							}
							else
							{
								//poly , 32B/64B
								size-=psz;
								if (psz==1)
								{
									TA_decoder::AppendPolyParam32((TA_PolyParamA*)data);
								}
								else
								{
									TA_decoder::AppendPolyParam64A((TA_PolyParamA*)data);
									data+=SZ32;
									TA_decoder::AppendPolyParam64B((TA_PolyParamB*)data);
								}
								data+=SZ32;
							}
						}
					}
					break;
					//32B
					//Sets Sprite info , and switches to ta_sprite_data function
				case ParamType_Sprite:
					{
						VerxexDataFP=ta_sprite1_data;
						TA_decoder::AppendSpriteParam((TA_SpriteParam*)data);
						data+=SZ32;
						size-=SZ32;
					}
					break;

					//Variable size
				case ParamType_Vertex_Parameter:
					//log ("vtx");
					{
						verify(VerxexDataFP!=0);
						u32 rv= VerxexDataFP(data,size);
						data+= rv;
						size-= rv;
					}
					break;

					//not handled
					//Assumed to be 32B
				default:
					{
						log("Unhadled parameter");
						data+=SZ32;
						size-=SZ32;
					}
					break;
				}
			}
			while(size);
			return ci;
		}
		//Rest shit
		static bool Init()
		{
			int i=0;
			ta_poly_data_lut[i++] = ta_poly_data<0,SZ32>;
			ta_poly_data_lut[i++] = ta_poly_data<1,SZ32>;
			ta_poly_data_lut[i++] = ta_poly_data<2,SZ32>;
			ta_poly_data_lut[i++] = ta_poly_data<3,SZ32>;
			ta_poly_data_lut[i++] = ta_poly_data<4,SZ32>;
			ta_poly_data_lut[i++] = ta_poly_data<5,SZ64>;
			ta_poly_data_lut[i++] = ta_poly_data<6,SZ64>;
			ta_poly_data_lut[i++] = ta_poly_data<7,SZ32>;
			ta_poly_data_lut[i++] = ta_poly_data<8,SZ32>;
			ta_poly_data_lut[i++] = ta_poly_data<9,SZ32>;
			ta_poly_data_lut[i++] = ta_poly_data<10,SZ32>;
			ta_poly_data_lut[i++] = ta_poly_data<11,SZ64>;
			ta_poly_data_lut[i++] = ta_poly_data<12,SZ64>;
			ta_poly_data_lut[i++] = ta_poly_data<13,SZ64>;
			ta_poly_data_lut[i++] = ta_poly_data<14,SZ64>;
			return true;
		}

		static void Term()
		{
			//ta_alloc_release_all();
		}

		static void Reset(bool manual)
		{
			//*couh* right ...
		}
		//called when writes are made to these registers..
		static void SoftReset()
		{
			ListIsFinished[0]=ListIsFinished[1]=ListIsFinished[2]=ListIsFinished[3]=ListIsFinished[4]=false;
			TA_decoder::SoftReset();
		}
		static void ListInit()
		{
			//reset TA input
			TaCmd=ta_main;

			ListIsFinished[0]=ListIsFinished[1]=ListIsFinished[2]=ListIsFinished[3]=ListIsFinished[4]=false;
			StripStarted=false;
			TA_decoder::ListInit();
		}
		static void ListCont()
		{
			ListIsFinished[0]=ListIsFinished[1]=ListIsFinished[2]=ListIsFinished[3]=ListIsFinished[4]=false;
			TA_decoder::ListCont();
		}
	};

	extern void Dma(u32 address,u32* data,u32 size);
}