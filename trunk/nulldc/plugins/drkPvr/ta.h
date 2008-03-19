#pragma once
#include "drkPvr.h"

namespace TASplitter
{
	void TA_ListCont();
	void TA_ListInit();
	void TA_SoftReset();
	extern void FASTCALL Dma(u32* data,u32 size);
    extern void FASTCALL SQ(u32* data);

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

	#include "ta_structs.h"

	typedef Ta_Dma* fastcall TaListFP(Ta_Dma* data,Ta_Dma* data_end);
	typedef u32 fastcall TaPolyParamFP(void* ptr);


	const HollyInterruptID ListEndInterrupt[5]=
	{
		holly_OPAQUE,
		holly_OPAQUEMOD,
		holly_TRANS,
		holly_TRANSMOD,
		holly_PUNCHTHRU
	};
	
	//Splitter function (normaly ta_dma_main , modified for split dma's)
	extern TaListFP* TaCmd;

	template<class TA_decoder>
	class FifoSplitter
	{
	public:
		//TA fifo state variables
		//Current List
		static u32 CurrentList; 
		//Vertex Handler function :)
		static TaListFP* VerxexDataFP;
		//finished lists
		static bool ListIsFinished[5];

		//splitter function lookup
		static u32 ta_type_lut[256];

		static bool StripStarted;

		static void fastcall ta_list_start(u32 new_list)
		{
			verify(CurrentList==ListType_None);
			verify(ListIsFinished[new_list]==false);
			//printf("Starting list %d\n",new_list);
			CurrentList=new_list;
			TA_decoder::StartList(CurrentList);
		}

		//part : 0 fill all data , 1 fill upper 32B , 2 fill lower 32B
		//Poly decoder , will be moved to pvr code
		template <u32 poly_type,u32 part,bool StripEnd>
		__forceinline
		static Ta_Dma* fastcall ta_handle_poly(Ta_Dma* data,Ta_Dma* data_end)
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
		if (part==0)	\
		{	\
		TA_decoder::AppendPolyVertex##num##B(&vp->vtx##num##B);\
		rv+=SZ32;	\
		} \
		else if (part==2)	\
		{	\
		TA_decoder::AppendPolyVertex##num##B((TA_Vertex##num##B*)data);\
		rv+=SZ32;	\
		} \
		}\
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
			return data+rv;
		};

		//Code Splitter/routers
		
		//helper function for dummy dma's.Handles 32B and then switches to ta_main for next data
		static Ta_Dma* fastcall ta_dummy_32(Ta_Dma* data,Ta_Dma* data_end)
		{
			TaCmd=ta_main;
			return data+SZ32;
		}
		static Ta_Dma* fastcall ta_modvolB_32(Ta_Dma* data,Ta_Dma* data_end)
		{
			TA_decoder::AppendModVolVertexB((TA_ModVolB*)data);
			TaCmd=ta_main;
			return data+SZ32;
		}
		
		static Ta_Dma* fastcall ta_mod_vol_data(Ta_Dma* data,Ta_Dma* data_end)
		{
			TA_VertexParam* vp=(TA_VertexParam*)data;
			if (data==data_end)
			{
				TA_decoder::AppendModVolVertexA(&vp->mvolA);
				//32B more needed , 32B done :)
				TaCmd=ta_modvolB_32;
				return data+SZ32;
			}
			else
			{
				//all 64B done
				TA_decoder::AppendModVolVertexA(&vp->mvolA);
				TA_decoder::AppendModVolVertexB(&vp->mvolB);
				return data+SZ64;
			}
		}
		static Ta_Dma* fastcall ta_spriteB_data(Ta_Dma* data,Ta_Dma* data_end)
		{
			//32B more needed , 32B done :)
			TaCmd=ta_main;
			
			TA_decoder::AppendSpriteVertexB((TA_Sprite1B*)data);

			return data+SZ32;
		}
		static Ta_Dma* fastcall ta_sprite_data(Ta_Dma* data,Ta_Dma* data_end)
		{
			if (data==data_end)
			{
				//32B more needed , 32B done :)
				TaCmd=ta_spriteB_data;
				
				TA_VertexParam* vp=(TA_VertexParam*)data;

				TA_decoder::AppendSpriteVertexA(&vp->spr1A);
				return data+SZ32;
			}
			else
			{
				TA_VertexParam* vp=(TA_VertexParam*)data;

				TA_decoder::AppendSpriteVertexA(&vp->spr1A);
				TA_decoder::AppendSpriteVertexB(&vp->spr1B);

				//all 64B done
				return data+SZ64;
			}
		}

		template <u32 poly_type,u32 poly_size>
		static Ta_Dma* fastcall ta_poly_data(Ta_Dma* data,Ta_Dma* data_end)
		{
			__assume(data<=data_end);
			if (StripStarted==false)
			{
				TA_decoder::StartPolyStrip();
				StripStarted=true;
			}

			while
				(
				poly_size==SZ32 ? 
				(data<=data_end)	//If SZ32 && we have 32 bytes left process em
				: 
				(data<data_end)		//If SZ32 && we have 32 bytes left skip em
				)
			{
				verify(data->pcw.ParaType==ParamType_Vertex_Parameter);

				ta_handle_poly<poly_type,0,false>(data,0);
		
				if (data->pcw.EndOfStrip)
					goto strip_end;
				
				data+=poly_size;
			}
			

			
			if ((poly_size!=SZ32) && (data==data_end))//32B part of 64B
			{
				ta_handle_poly<poly_type,1,false>(data,0);
				if (data->pcw.EndOfStrip)
					TaCmd=ta_handle_poly<poly_type,2,true>;//end strip after part B is  done :)
				else
					TaCmd=ta_handle_poly<poly_type,2,false>;
				data+=SZ32;
			}
			
			return data;

strip_end:
			StripStarted=false;
			TA_decoder::EndPolyStrip();
			return data+SZ32;
		}

				
		static void fastcall AppendPolyParam2Full(Ta_Dma* pp)
		{
			TA_decoder::AppendPolyParam2A((TA_PolyParam2A*)&pp[0]);
			TA_decoder::AppendPolyParam2B((TA_PolyParam2B*)&pp[1]);
		}

		static void fastcall AppendPolyParam4Full(Ta_Dma* pp)
		{
			TA_decoder::AppendPolyParam4A((TA_PolyParam4A*)&pp[0]);
			TA_decoder::AppendPolyParam4B((TA_PolyParam4B*)&pp[1]);
		}
		//Second part of poly data
		template <int t>
		static Ta_Dma* fastcall ta_poly_B_32(Ta_Dma* data,Ta_Dma* data_end)
		{
			if (t==2)
				TA_decoder::AppendPolyParam2B((TA_PolyParam2B*)data);
			else
				TA_decoder::AppendPolyParam4B((TA_PolyParam4B*)data);
	
			TaCmd=ta_main;
			return data+SZ32;
		}

public:
#define GROOP_EN() if (data->pcw.Group_En){ TA_decoder::TileClipMode(data->pcw.User_Clip);}
		static Ta_Dma* fastcall ta_main(Ta_Dma* data,Ta_Dma* data_end)
		{
			do
			{
				switch (data->pcw.ParaType)
				{
					//Control parameter
					//32Bw3
				case ParamType_End_Of_List:
					{
						if (CurrentList==ListType_None)
						{
							CurrentList=data->pcw.ListType;
							//printf("End_Of_List : list error\n");
						}
						else
						{
							//end of list should be all 0's ...
							TA_decoder::EndList(CurrentList);//end a list olny if it was realy started
						}

						//printf("End list %X\n",CurrentList);
						params.RaiseInterrupt(ListEndInterrupt[CurrentList]);
						ListIsFinished[CurrentList]=true;
						CurrentList=ListType_None;
						VerxexDataFP=0;
						data+=SZ32;
					}
					break;
					//32B
				case ParamType_User_Tile_Clip:
					{
						TA_decoder::SetTileClip(data->data_32[3]&63,data->data_32[4]&31,data->data_32[5]&63,data->data_32[6]&31);
						//*couh* ignore it :p
						data+=SZ32;
					}
					break;	
					//32B
				case ParamType_Object_List_Set:
					{
						die("ParamType_Object_List_Set");
						//*couh* ignore it :p
						data+=SZ32;
					}
					break;

					//Global Parameter
					//ModVolue :32B
					//PolyType :32B/64B
				case ParamType_Polygon_or_Modifier_Volume:
					{
						GROOP_EN();
						//Yep , C++ IS lame & limited
						#include "ta_const_df.h"
						if (CurrentList==ListType_None)
							ta_list_start(data->pcw.ListType);	//start a list ;)

						if (IsModVolList(CurrentList))
						{	//accept mod data
							TA_decoder::StartModVol((TA_ModVolParam*)data);
							VerxexDataFP=ta_mod_vol_data;
							data+=SZ32;
						}
						else
						{

							u32 uid=ta_type_lut[data->pcw.obj_ctrl];
							u32 psz=uid>>30;
							u32 pdid=(u8)(uid);
							u32 ppid=(u8)(uid>>8);

							VerxexDataFP=ta_poly_data_lut[pdid];
							verify(StripStarted==false);
							
							if (data != data_end || psz==1)
							{
								//poly , 32B/64B
								ta_poly_param_lut[ppid](data);
								data+=psz;
							}
							else
							{
								//TA_decoder::AppendPolyParam64A((TA_PolyParamA*)data);
								//64b , first part
								ta_poly_param_a_lut[ppid](data);
								//Handle next 32B ;)
								TaCmd=ta_poly_param_b_lut[ppid];
								data+=SZ32;
							}
						}
					}
					break;
					//32B
					//Sets Sprite info , and switches to ta_sprite_data function
				case ParamType_Sprite:
					{
						GROOP_EN();
						if (CurrentList==ListType_None)
							ta_list_start(data->pcw.ListType);	//start a list ;)

						VerxexDataFP=ta_sprite_data;
						//printf("Sprite \n");
						TA_decoder::AppendSpriteParam((TA_SpriteParam*)data);
						data+=SZ32;
					}
					break;

					//Variable size
				case ParamType_Vertex_Parameter:
					//log ("vtx");
					{
						//printf("VTX:0x%08X\n",VerxexDataFP);
						verify(VerxexDataFP!=0);
						data=VerxexDataFP(data,data_end);
					}
					break;

					//not handled
					//Assumed to be 32B
				case 3:
				case 6:
					{
						die("Unhadled parameter");
						data+=SZ32;
					}
					break;
				}
			}
			while(data<=data_end);
			return data;
		}
		//Rest shit
		FifoSplitter()
		{
			Init();
		}
		/*
		Volume,Col_Type,Texture,Offset,Gouraud,16bit_UV
		
		0	0	0	(0)	x	invalid	Polygon Type 0	Polygon Type 0
		0	0	1	x	x	0		Polygon Type 0	Polygon Type 3
		0	0	1	x	x	1		Polygon Type 0	Polygon Type 4

		0	1	0	(0)	x	invalid	Polygon Type 0	Polygon Type 1
		0	1	1	x	x	0		Polygon Type 0	Polygon Type 5
		0	1	1	x	x	1		Polygon Type 0	Polygon Type 6

		0	2	0	(0)	x	invalid	Polygon Type 1	Polygon Type 2
		0	2	1	0	x	0		Polygon Type 1	Polygon Type 7
		0	2	1	0	x	1		Polygon Type 1	Polygon Type 8
		0	2	1	1	x	0		Polygon Type 2	Polygon Type 7
		0	2	1	1	x	1		Polygon Type 2	Polygon Type 8

		0	3	0	(0)	x	invalid	Polygon Type 0	Polygon Type 2
		0	3	1	x	x	0		Polygon Type 0	Polygon Type 7
		0	3	1	x	x	1		Polygon Type 0	Polygon Type 8

		1	0	0	(0)	x	invalid	Polygon Type 3	Polygon Type 9
		1	0	1	x	x	0		Polygon Type 3	Polygon Type 11
		1	0	1	x	x	1		Polygon Type 3	Polygon Type 12

		1	2	0	(0)	x	invalid	Polygon Type 4	Polygon Type 10
		1	2	1	x	x	0		Polygon Type 4	Polygon Type 13
		1	2	1	x	x	1		Polygon Type 4	Polygon Type 14

		1	3	0	(0)	x	invalid	Polygon Type 3	Polygon Type 10
		1	3	1	x	x	0		Polygon Type 3	Polygon Type 13
		1	3	1	x	x	1		Polygon Type 3	Polygon Type 14
		
		Sprites :
		(0)	(0)	0	(0)	(0)	invalid	Sprite	Sprite Type 0
		(0)	(0)	1	x	(0)	(1)		Sprite	Sprite Type 1

		*/
		//helpers 0-14
		static u32 fastcall poly_data_type_id(PCW pcw)
		{
			if (pcw.Texture)
			{
				//textured
				if (pcw.Volume==0)
				{	//single volume
					if (pcw.Col_Type==0)
					{
						if (pcw.UV_16bit==0)
							return 3;					//(Textured, Packed Color , 32b uv)
						else
							return 4;					//(Textured, Packed Color , 16b uv)
					}
					else if (pcw.Col_Type==1)
					{
						if (pcw.UV_16bit==0)
							return 5;					//(Textured, Floating Color , 32b uv)
						else
							return 6;					//(Textured, Floating Color , 16b uv)
					}
					else
					{
						if (pcw.UV_16bit==0)
							return 7;					//(Textured, Intensity , 32b uv)
						else
							return 8;					//(Textured, Intensity , 16b uv)
					}
				}
				else
				{
					//two volumes
					if (pcw.Col_Type==0)
					{
						if (pcw.UV_16bit==0)
							return 11;					//(Textured, Packed Color, with Two Volumes)	

						else
							return 12;					//(Textured, Packed Color, 16bit UV, with Two Volumes)

					}
					else if (pcw.Col_Type==1)
					{
						//die ("invalid");
						return 0xFFFFFFFF;
					}
					else
					{
						if (pcw.UV_16bit==0)
							return 13;					//(Textured, Intensity, with Two Volumes)	

						else
							return 14;					//(Textured, Intensity, 16bit UV, with Two Volumes)
					}
				}
			}
			else
			{
				//non textured
				if (pcw.Volume==0)
				{	//single volume
					if (pcw.Col_Type==0)
						return 0;						//(Non-Textured, Packed Color)
					else if (pcw.Col_Type==1)
						return 1;						//(Non-Textured, Floating Color)
					else
						return 2;						//(Non-Textured, Intensity)
				}
				else
				{
					//two volumes
					if (pcw.Col_Type==0)
						return 9;						//(Non-Textured, Packed Color, with Two Volumes)
					else if (pcw.Col_Type==1)
					{
						//die ("invalid");
						return 0xFFFFFFFF;
					}
					else
						return 10;						//(Non-Textured, Intensity, with Two Volumes)
				}
			}
			//dbgbreak;
			return 0xFFFFFFFF;
		}
		//0-4 | 0x80
		static u32 fastcall poly_header_type_size(PCW pcw)
		{
			if (pcw.Volume == 0)
			{
				if ( pcw.Col_Type<2 ) //0,1
				{
					return 0  | 0;	//Polygon Type 0 -- SZ32
				}
				else if ( pcw.Col_Type == 2 )
				{
					if (pcw.Texture)
					{
						if (pcw.Offset)
						{
							return 2 | 0x80;	//Polygon Type 2 -- SZ64
						}
						else
						{
							return 1 | 0;	//Polygon Type 1 -- SZ32
						}
					}
					else
					{
						return 1 | 0;	//Polygon Type 1 -- SZ32
					}
				}
				else	//col_type ==3
				{
					return 0 | 0;	//Polygon Type 0 -- SZ32
				}
			}
			else
			{
				if ( pcw.Col_Type==0 ) //0
				{
					return 3 | 0;	//Polygon Type 3 -- SZ32
				}
				else if ( pcw.Col_Type==2 ) //2
				{
					return 4 | 0x80;	//Polygon Type 4 -- SZ64
				}
				else if ( pcw.Col_Type==3 ) //3
				{
					return 3 | 0;	//Polygon Type 3 -- SZ32
				}
				else
				{
					return 0xFFDDEEAA;//die ("data->pcw.Col_Type==1 && volume ==1");
				}
			}
		}


		static bool Init()
		{
			for (int i=0;i<256;i++)
			{
				PCW pcw;
				pcw.obj_ctrl=i;
				u32 rv=	poly_data_type_id(pcw);
				u32 type= poly_header_type_size(pcw);

				if (type& 0x80)
					rv|=(SZ64<<30);
				else
					rv|=(SZ32<<30);

				rv|=(type&0x7F)<<8;

				ta_type_lut[i]=rv;
			}

			ListIsFinished[0]=ListIsFinished[1]=ListIsFinished[2]=ListIsFinished[3]=ListIsFinished[4]=false;
			StripStarted=false;
			return true;
		}

		~FifoSplitter()
		{
			Term();
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
			TA_SoftReset();
			ListIsFinished[0]=ListIsFinished[1]=ListIsFinished[2]=ListIsFinished[3]=ListIsFinished[4]=false;
			StripStarted=false;
			CurrentList=ListType_None;

			TA_decoder::SoftReset();
		}
		static void ListInit()
		{
			TA_ListInit();
			//reset TA input
			TaCmd=ta_main;

			ListIsFinished[0]=ListIsFinished[1]=ListIsFinished[2]=ListIsFinished[3]=ListIsFinished[4]=false;
			
			StripStarted=false;
			if (CurrentList!=ListType_None)
			{
				//StripStarted should be checked ?
				TA_decoder::EndList(CurrentList);
				CurrentList=ListType_None;
			}

			TA_decoder::ListInit();
		}
		static void ListCont()
		{
			TA_ListCont();
			ListIsFinished[0]=ListIsFinished[1]=ListIsFinished[2]=ListIsFinished[3]=ListIsFinished[4]=false;
			TA_decoder::ListCont();
		}
	};


	//Well , olny in a C++ world you have to do smth like that ...
	template <class TA_decoder> u32 FifoSplitter<TA_decoder> ::CurrentList;
	template <class TA_decoder> TaListFP* FifoSplitter<TA_decoder> ::VerxexDataFP;
	template <class TA_decoder> bool FifoSplitter<TA_decoder> ::ListIsFinished[5];
	template <class TA_decoder> u32 FifoSplitter<TA_decoder> ::ta_type_lut[256];
	template <class TA_decoder> bool FifoSplitter<TA_decoder> ::StripStarted;

	template<class TA_decoder>
	class FifoSplitterEx
	{
	public:
		//TA fifo state variables
		//Current List
		static u32 CurrentList; 
		//Vertex Handler function :)
		static TaListFP* VerxexDataFP;
		//finished lists
		static bool ListIsFinished[5];

		//splitter function lookup
		static u32 ta_type_lut[256];

		//helper function for dummy dma's.Handles 32B and then switches to ta_main for next data
		static Ta_Dma* fastcall ta_dummy_32(Ta_Dma* data,Ta_Dma* data_end)
		{
			TaCmd=ta_main;
			return data+SZ32;
		}
		static Ta_Dma* fastcall ta_modvolB_32(Ta_Dma* data,Ta_Dma* data_end)
		{
			//TA_decoder::AppendModVolVertexB((TA_ModVolB*)data);
			TaCmd=ta_main;
			return data+SZ32;
		}
		//Second part of poly data
		template <int t>
		static Ta_Dma* fastcall ta_poly_B_32(Ta_Dma* data,Ta_Dma* data_end)
		{
			if (t==2)
				TA_decoder::AppendPolyParam2B((TA_PolyParam2B*)data);
			else
				TA_decoder::AppendPolyParam4B((TA_PolyParam4B*)data);
	
			TaCmd=ta_main;
			return data+SZ32;
		}

		static Ta_Dma* fastcall ta_poly_data_B_32(Ta_Dma* data,Ta_Dma* data_end)
		{
			TA_decoder::AppendPolyVertexData32Part(data);
			TaCmd=ta_main;
			return data+SZ32;
		}

		//Code Splitter/routers
		static Ta_Dma* fastcall ta_mod_vol_data(Ta_Dma* data,Ta_Dma* data_end)
		{
			TA_VertexParam* vp=(TA_VertexParam*)data;
			if (data==data_end)
			{
				//TA_decoder::AppendModVolVertexA(&vp->mvolA);
				//32B more needed , 32B done :)
				TaCmd=ta_modvolB_32;
				return data+SZ32;
			}
			else
			{
				//all 64B done
				//TA_decoder::AppendModVolVertexA(&vp->mvolA);
				//TA_decoder::AppendModVolVertexB(&vp->mvolB);
				return data+SZ64;
			}
		}
		static Ta_Dma* fastcall ta_spriteB_data(Ta_Dma* data,Ta_Dma* data_end)
		{
			//32B more needed , 32B done :)
			TaCmd=ta_main;
			
		//	TA_decoder::AppendSpriteVertexB((TA_Sprite1B*)data);

			return data+SZ32;
		}
		static Ta_Dma* fastcall ta_sprite_data(Ta_Dma* data,Ta_Dma* data_end)
		{
			if (data==data_end)
			{
				//32B more needed , 32B done :)
				TaCmd=ta_spriteB_data;
				
				TA_VertexParam* vp=(TA_VertexParam*)data;

				//TA_decoder::AppendSpriteVertexA(&vp->spr1A);
				return data+SZ32;
			}
			else
			{
				TA_VertexParam* vp=(TA_VertexParam*)data;

				//TA_decoder::AppendSpriteVertexA(&vp->spr1A);
				//TA_decoder::AppendSpriteVertexB(&vp->spr1B);

				//all 64B done
				return data+SZ64;
			}
		}


		//Transfer polygon data
		template <u32 poly_size,bool FirstTransfer>
		static Ta_Dma* fastcall ta_poly_data(Ta_Dma* data,Ta_Dma* data_end)
		{
			do
			{
				if (data->pcw.full<0xE0000000)
					goto strip_end;

				if (poly_size==SZ32)
					TA_decoder::AppendPolyVertexData32(data);
				else
					TA_decoder::AppendPolyVertexData(data);		

				data+=poly_size;
			}
			while
				(
				poly_size==SZ32 ? 
				(data<=data_end)	//If SZ32 && we have 32 bytes left process em
				: 
				(data<data_end)		//If SZ32 && we have 32 bytes left skip em
				);

			
			if ((poly_size==SZ64) && (data==data_end))//32B part of 64B
			{
				TA_decoder::AppendPolyVertexData32(data);
				TaCmd=ta_poly_data_B_32;
				data+=SZ32;
			}
			else if (FirstTransfer==true)
			{
				//strip didnt finish at once.. continue it later :P
				VerxexDataFP=TaCmd=ta_poly_data<poly_size,false>;
			}
			
			return data;

strip_end:
			if (FirstTransfer!=true)	//if strip wasnt started this time
			{
				VerxexDataFP=ta_poly_data<poly_size,false>;
				TaCmd=ta_main;
			}

			return data;
		}

		/*
		Volume,Col_Type,Texture,Offset,Gouraud,16bit_UV
		
		0	0	0	(0)	x	invalid	Polygon Type 0	Polygon Type 0
		0	0	1	x	x	0		Polygon Type 0	Polygon Type 3
		0	0	1	x	x	1		Polygon Type 0	Polygon Type 4

		0	1	0	(0)	x	invalid	Polygon Type 0	Polygon Type 1
		0	1	1	x	x	0		Polygon Type 0	Polygon Type 5
		0	1	1	x	x	1		Polygon Type 0	Polygon Type 6

		0	2	0	(0)	x	invalid	Polygon Type 1	Polygon Type 2
		0	2	1	0	x	0		Polygon Type 1	Polygon Type 7
		0	2	1	0	x	1		Polygon Type 1	Polygon Type 8
		0	2	1	1	x	0		Polygon Type 2	Polygon Type 7
		0	2	1	1	x	1		Polygon Type 2	Polygon Type 8

		0	3	0	(0)	x	invalid	Polygon Type 0	Polygon Type 2
		0	3	1	x	x	0		Polygon Type 0	Polygon Type 7
		0	3	1	x	x	1		Polygon Type 0	Polygon Type 8

		1	0	0	(0)	x	invalid	Polygon Type 3	Polygon Type 9
		1	0	1	x	x	0		Polygon Type 3	Polygon Type 11
		1	0	1	x	x	1		Polygon Type 3	Polygon Type 12

		1	2	0	(0)	x	invalid	Polygon Type 4	Polygon Type 10
		1	2	1	x	x	0		Polygon Type 4	Polygon Type 13
		1	2	1	x	x	1		Polygon Type 4	Polygon Type 14

		1	3	0	(0)	x	invalid	Polygon Type 3	Polygon Type 10
		1	3	1	x	x	0		Polygon Type 3	Polygon Type 13
		1	3	1	x	x	1		Polygon Type 3	Polygon Type 14
		
		Sprites :
		(0)	(0)	0	(0)	(0)	invalid	Sprite	Sprite Type 0
		(0)	(0)	1	x	(0)	(1)		Sprite	Sprite Type 1

		*/
		//helpers 0-14
		static u32 fastcall poly_data_type_id(PCW pcw)
		{
			if (pcw.Texture)
			{
				//textured
				if (pcw.Volume==0)
				{	//single volume
					if (pcw.Col_Type==0)
					{
						if (pcw.UV_16bit==0)
							return 3;					//(Textured, Packed Color , 32b uv)
						else
							return 4;					//(Textured, Packed Color , 16b uv)
					}
					else if (pcw.Col_Type==1)
					{
						if (pcw.UV_16bit==0)
							return 5;					//(Textured, Floating Color , 32b uv)
						else
							return 6;					//(Textured, Floating Color , 16b uv)
					}
					else
					{
						if (pcw.UV_16bit==0)
							return 7;					//(Textured, Intensity , 32b uv)
						else
							return 8;					//(Textured, Intensity , 16b uv)
					}
				}
				else
				{
					//two volumes
					if (pcw.Col_Type==0)
					{
						if (pcw.UV_16bit==0)
							return 11;					//(Textured, Packed Color, with Two Volumes)	

						else
							return 12;					//(Textured, Packed Color, 16bit UV, with Two Volumes)

					}
					else if (pcw.Col_Type==1)
					{
						//die ("invalid");
						return 0xFFFFFFFF;
					}
					else
					{
						if (pcw.UV_16bit==0)
							return 13;					//(Textured, Intensity, with Two Volumes)	

						else
							return 14;					//(Textured, Intensity, 16bit UV, with Two Volumes)
					}
				}
			}
			else
			{
				//non textured
				if (pcw.Volume==0)
				{	//single volume
					if (pcw.Col_Type==0)
						return 0;						//(Non-Textured, Packed Color)
					else if (pcw.Col_Type==1)
						return 1;						//(Non-Textured, Floating Color)
					else
						return 2;						//(Non-Textured, Intensity)
				}
				else
				{
					//two volumes
					if (pcw.Col_Type==0)
						return 9;						//(Non-Textured, Packed Color, with Two Volumes)
					else if (pcw.Col_Type==1)
					{
						//die ("invalid");
						return 0xFFFFFFFF;
					}
					else
						return 10;						//(Non-Textured, Intensity, with Two Volumes)
				}
			}
			//dbgbreak;
			return 0xFFFFFFFF;
		}
		//0-4 | 0x80
		static u32 fastcall poly_header_type_size(PCW pcw)
		{
			if (pcw.Volume == 0)
			{
				if ( pcw.Col_Type<2 ) //0,1
				{
					return 0  | 0;	//Polygon Type 0 -- SZ32
				}
				else if ( pcw.Col_Type == 2 )
				{
					if (pcw.Texture)
					{
						if (pcw.Offset)
						{
							return 2 | 0x80;	//Polygon Type 2 -- SZ64
						}
						else
						{
							return 1 | 0;	//Polygon Type 1 -- SZ32
						}
					}
					else
					{
						return 1 | 0;	//Polygon Type 1 -- SZ32
					}
				}
				else	//col_type ==3
				{
					return 0 | 0;	//Polygon Type 0 -- SZ32
				}
			}
			else
			{
				if ( pcw.Col_Type==0 ) //0
				{
					return 3 | 0;	//Polygon Type 3 -- SZ32
				}
				else if ( pcw.Col_Type==2 ) //2
				{
					return 4 | 0x80;	//Polygon Type 4 -- SZ64
				}
				else if ( pcw.Col_Type==3 ) //3
				{
					return 3 | 0;	//Polygon Type 3 -- SZ32
				}
				else
				{
					return 0xFFDDEEAA;//die ("data->pcw.Col_Type==1 && volume ==1");
				}
			}
		}


		static void fastcall ta_list_start(u32 new_list)
		{
			verify(CurrentList==ListType_None);
			verify(ListIsFinished[new_list]==false);
			//printf("Starting list %d\n",new_list);
			CurrentList=new_list;
			TA_decoder::StartList(CurrentList);
		}
		
		static void fastcall AppendPolyParam2Full(Ta_Dma* pp)
		{
			TA_decoder::AppendPolyParam2A((TA_PolyParam2A*)&pp[0]);
			TA_decoder::AppendPolyParam2B((TA_PolyParam2B*)&pp[1]);
		}

		static void fastcall AppendPolyParam4Full(Ta_Dma* pp)
		{
			TA_decoder::AppendPolyParam4A((TA_PolyParam4A*)&pp[0]);
			TA_decoder::AppendPolyParam4B((TA_PolyParam4B*)&pp[1]);
		}

public:
#define GROOP_EN() if (data->pcw.Group_En){ TA_decoder::TileClipMode(data->pcw.User_Clip);}
		static Ta_Dma* fastcall ta_main(Ta_Dma* data,Ta_Dma* data_end)
		{
			//u32 ci=size;
			do
			{
				//Variable size
				switch (data->pcw.ParaType)
				{
					default:__assume(0);
					//Control parameter
					//32B
				case ParamType_End_Of_List:
					{
						if (VerxexDataFP)
						{
						//	TA_decoder::EndParam();
							VerxexDataFP=0;
						}
						if (CurrentList==ListType_None)
						{
							CurrentList=data->pcw.ListType;
							//printf("End_Of_List : list error\n");
						}
						else
						{
							//end of list should be all 0's ...
							TA_decoder::EndList(CurrentList);//end a list olny if it was realy started
						}

						//printf("End list %X\n",CurrentList);
						params.RaiseInterrupt(ListEndInterrupt[CurrentList]);
						ListIsFinished[CurrentList]=true;
						CurrentList=ListType_None;
						data+=SZ32;
					}
					break;
					//32B
				case ParamType_User_Tile_Clip:
					{
						TA_decoder::SetTileClip(data->data_32[3]&63,data->data_32[4]&31,data->data_32[5]&63,data->data_32[6]&31);
						//*couh* ignore it :p
						data+=SZ32;
					}
					break;	
					//32B
				case ParamType_Object_List_Set:
					{
						die("ParamType_Object_List_Set");
						//*couh* ignore it :p
						data+=SZ32;
					}
					break;

					//Global Parameter
					//ModVolue :32B
					//PolyType :32B/64B
				case ParamType_Polygon_or_Modifier_Volume:
					{
						//this is here to make up for C++'s limitations
						//Yep , C++ IS lame & limited
						static TaListFP* ta_poly_data_lut[2] = 
						{
							ta_poly_data<SZ32,true>,
							ta_poly_data<SZ64,true>,
						};
						//32/64b , full
						static TaPolyParamFP* ta_poly_param_lut[5]=
						{
							(TaPolyParamFP*)TA_decoder::AppendPolyParam0,
							(TaPolyParamFP*)TA_decoder::AppendPolyParam1,
							(TaPolyParamFP*)AppendPolyParam2Full,
							(TaPolyParamFP*)TA_decoder::AppendPolyParam3,
							(TaPolyParamFP*)AppendPolyParam4Full
						};
						//64b , first part
						static TaPolyParamFP* ta_poly_param_a_lut[5]=
						{
							(TaPolyParamFP*)0,
							(TaPolyParamFP*)0,
							(TaPolyParamFP*)TA_decoder::AppendPolyParam2A,
							(TaPolyParamFP*)0,
							(TaPolyParamFP*)TA_decoder::AppendPolyParam4A
						};

						//64b , , second part
						static TaListFP* ta_poly_param_b_lut[5]=
						{
							(TaListFP*)0,
							(TaListFP*)0,
							(TaListFP*)ta_poly_B_32<2>,
							(TaListFP*)0,
							(TaListFP*)ta_poly_B_32<4>
						};

						GROOP_EN();
						if (CurrentList==ListType_None)
							ta_list_start(data->pcw.ListType);	//start a list ;)
						if (!IsModVolList(CurrentList))
						{

							u32 uid=ta_type_lut[data->pcw.obj_ctrl];
							u32 pdsz=(u8)(uid)&1;
							u32 ppsz=(u8)(uid)&(32);
							u32 ppid=(u8)(uid>>8);

							VerxexDataFP=ta_poly_data_lut[pdsz];
							
							if (ppsz==0 || data != data_end)
							{
								ta_poly_param_lut[ppid](data);
								data=(Ta_Dma*) &((u8*)data)[ppsz+32];
							}
							else
							{
								//64b , first part
								ta_poly_param_a_lut[ppid](data);
								//Handle next 32B ;)
								TaCmd=ta_poly_param_b_lut[ppid];
								data+=SZ32;
								return data;
							}
						}
						else
						{	//accept mod data
							//TA_decoder::StartModVol((TA_ModVolParam*)data);
							VerxexDataFP=ta_mod_vol_data;
							data+=SZ32;
						}
					}
					break;
					//32B
					//Sets Sprite info , and switches to ta_sprite_data function
				case ParamType_Sprite:
					{
						GROOP_EN();
						if (CurrentList==ListType_None)
							ta_list_start(data->pcw.ListType);	//start a list ;)

						VerxexDataFP=ta_sprite_data;
						//printf("Sprite \n");
						//TA_decoder::AppendSpriteParam((TA_SpriteParam*)data);
						data+=SZ32;
					}
					break;

				case ParamType_Vertex_Parameter:
					{
						verify(VerxexDataFP!=0);
						data=VerxexDataFP(data,data_end);
					}
					break;
					//not handled
					//Assumed to be 32B
				case 3:
				case 6:
					{
						die("Unhadled parameter");
						data+=SZ32;
					}
					break;
				}
			}
			while(data<=data_end);
			return data;
		}
		//Rest shit
		FifoSplitterEx()
		{
			Init();
		}
		static bool Init()
		{
			for (int i=0;i<256;i++)
			{
				PCW pcw;
				pcw.obj_ctrl=i;
				u32 rv=0;
				rv=	poly_data_type_id(pcw);
				if (rv==5||rv==6||rv==11||rv==12||rv==13||rv==14)
					rv=1;
				else
					rv=0;
				u32 type= poly_header_type_size(pcw);

				if (type& 0x80)
					rv|=32;
				
				rv|=(type&0x7F)<<8;

				ta_type_lut[i]=rv;
			}

			ListIsFinished[0]=ListIsFinished[1]=ListIsFinished[2]=ListIsFinished[3]=ListIsFinished[4]=false;
			return true;
		}

		~FifoSplitterEx()
		{
			Term();
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
			TA_SoftReset();
			ListIsFinished[0]=ListIsFinished[1]=ListIsFinished[2]=ListIsFinished[3]=ListIsFinished[4]=false;
			CurrentList=ListType_None;

			TA_decoder::SoftReset();
		}
		static void ListInit()
		{
			TA_ListInit();
			//reset TA input
			TaCmd=ta_main;

			ListIsFinished[0]=ListIsFinished[1]=ListIsFinished[2]=ListIsFinished[3]=ListIsFinished[4]=false;
			CurrentList=ListType_None;

			TA_decoder::ListInit();
		}
		static void ListCont()
		{
			TA_ListCont();
			ListIsFinished[0]=ListIsFinished[1]=ListIsFinished[2]=ListIsFinished[3]=ListIsFinished[4]=false;
			TA_decoder::ListCont();
		}
	};



	//Well , olny in a C++ world you have to do smth like that ...
	template <class TA_decoder> u32 FifoSplitterEx<TA_decoder> ::CurrentList;
	template <class TA_decoder> TaListFP* FifoSplitterEx<TA_decoder> ::VerxexDataFP;
	template <class TA_decoder> bool FifoSplitterEx<TA_decoder> ::ListIsFinished[5];
	template <class TA_decoder> u32 FifoSplitterEx<TA_decoder> ::ta_type_lut[256];
}