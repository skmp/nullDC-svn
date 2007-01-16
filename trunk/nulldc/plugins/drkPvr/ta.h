#pragma once
#include "drkPvr.h"

namespace TASplitter
{
	//void TaWrite(u32* data);
	void TADma(u32 address,u32* data,u32 size);

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

	typedef u32 fastcall TaListFP(Ta_Dma* data,u32 size);
	typedef u32 fastcall TaPolyParamFP(void* ptr);


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
	//extern TaListFP* ta_poly_data_lut[15];
	//extern TaPolyParamFP* ta_poly_param_lut[5];
	//extern TaPolyParamFP* ta_poly_param_a_lut[5];
	//extern TaPolyParamFP* ta_poly_param_b_lut[5];
	extern u32 ta_type_lut[256];

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
			//TA_decoder::AppendPolyParam64B((TA_PolyParamB*)data);
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
			TA_decoder::AppendSpriteVertex1B((TA_Sprite1B*)data);
			return SZ32;
		}
		static u32 fastcall ta_sprite1_data(Ta_Dma* data,u32 size)
		{
			if (size==1)
			{
				//32B more needed , 32B done :)
				TaCmd=ta_sprite1B_data;
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
						//die ("invalid");
						return 0xFFFFFFFF;
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
		
		static u32 fastcall poly_header_type_size(Ta_Dma* data)
		{
			if (data->pcw.Volume == 0)
			{
				if ( data->pcw.Col_Type<2 ) //0,1
				{
					return SZ32;	//Polygon Type 0
				}
				else if ( data->pcw.Col_Type == 2 )
				{
					if (data->pcw.Texture)
					{
						if (data->pcw.Offset)
						{
							return SZ64;	//Polygon Type 2
						}
						else
						{
							return SZ32;	//Polygon Type 1
						}
					}
					else
					{
						return SZ32;	//Polygon Type 1
					}
				}
				else	//col_type ==3
				{
					return SZ32;	//Polygon Type 0
				}
			}
			else
			{
				if ( data->pcw.Col_Type==0 ) //0
				{
					return SZ32;	//Polygon Type 3
				}
				else if ( data->pcw.Col_Type==2 ) //2
				{
					return SZ64;	//Polygon Type 4
				}
				else if ( data->pcw.Col_Type==3 ) //3
				{
					return SZ32;	//Polygon Type 3
				}
				else
				{
					return 0xFFFFFFFF;//die ("data->pcw.Col_Type==1 && volume ==1");
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
						size-=SZ32;
					}
					break;
					//32B
				case ParamType_User_Tile_Clip:
					{
						//printf("TILECLIP\n");
						//*couh* ignore it :p
						data+=SZ32;
						size-=SZ32;
					}
					break;	
					//32B
				case ParamType_Object_List_Set:
					{
						die("ParamType_Object_List_Set");
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
						//Yep , C++ IS lame & limited
						#include "ta_const_df.h"
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

							u32 uid=ta_type_lut[data->pcw.obj_ctrl];
							u32 psz=uid>>30;
							u32 pdid=(u8)(uid);
							u32 ppid=(u8)(uid>>8);

							VerxexDataFP=ta_poly_data_lut[pdid];
							verify(StripStarted==false);
							
							if (psz>size)
							{
								//TA_decoder::AppendPolyParam64A((TA_PolyParamA*)data);
								//64b , first part
								ta_poly_param_a_lut[ppid](data);
								//Handle next 32B ;)
								TaCmd=ta_poly_B_32;//ta_poly_param_a_lut[ppid] -> gota fix dat
								data+=SZ32;
								size-=SZ32;
							}
							else
							{
								//poly , 32B/64B
								size-=psz;
								ta_poly_param_lut[ppid](data);
								data+=psz;
							}
						}
					}
					break;
					//32B
					//Sets Sprite info , and switches to ta_sprite_data function
				case ParamType_Sprite:
					{
						VerxexDataFP=ta_sprite1_data;
						//printf("Sprite \n");
						TA_decoder::AppendSpriteParam((TA_SpriteParam*)data);
						data+=SZ32;
						size-=SZ32;
					}
					break;

					//Variable size
				case ParamType_Vertex_Parameter:
					//log ("vtx");
					{
						//printf("VTX:0x%08X\n",VerxexDataFP);
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
						die("Unhadled parameter");
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
		FifoSplitter()
		{
			Init();
		}
		static bool Init()
		{
			for (int i=0;i<256;i++)
			{
				Ta_Dma t;
				t.pcw.obj_ctrl=i;
				ta_type_lut[i]=	poly_data_type_id(&t) | 
								(poly_header_type_size(&t)<<30) |
								((poly_header_type_size(&t)==1?0:4)<<8);

			}
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