#include "ta.h"
//Tile Accelerator state machine
#include "ta_alloc.h"

List<TaVertex> verts_pv;

List<TaVertexList> vertlists_pv;

List<TaGlobalParam> global_param_op_pv[14];
List<TaGlobalParam> global_param_tr_pv[14];

u32 CurrentList=ListType_None;
 
TaListFP* TaCmd=0;
InterruptID ListEndInterrupt[5]=
{
	InterruptID::holly_OPAQUE,
	InterruptID::holly_OPAQUEMOD,
	InterruptID::holly_TRANS,
	InterruptID::holly_TRANSMOD,
	InterruptID::holly_PUNCHTHRU
};

bool ListIsFinished[5]={false,false,false,false,false};

u32 fastcall ta_main(Ta_Dma* data,u32 size);

TaGlobalParam*	CurrentGP;
TaVertexList*	CurrentVL;
TaVertex*		temp_lastvert;
void fastcall SetUserClip(Ta_Dma* param)
{
	//TODO : Add support for user cliping
}

void fastcall ta_list_start(u32 new_list)
{
	verify(ListIsFinished[new_list]==false);
	CurrentList=new_list;
}
#define SZ32 1
#define SZ64 2

void* part_data=0;
template<TaListFP* nl>
u32 fastcall ta_dummy_32(Ta_Dma* data,u32 size)
{
	TaCmd=nl;
	return SZ32;
}
f32 f16(u16 v)
{
	u32 z=v<<16;
	return *(f32*)&z;
}
f32 z_min;
f32 z_max;
//part is either 0 either 1
//part : 0 fill all data , 1 fill upper 32B , 2 fill lower 32B
template <u32 poly_type,u32 part>
u32 fastcall ta_handle_poly(Ta_Dma* data,u32 size)
{
	TaVertex* cv;
	VertexParam* vp=(VertexParam*)data;
	u32 rv=0;
#define ver_32B_def \
			cv = verts_pv.Append();\
			cv->xyz[0]=((float*)data)[1];\
			cv->xyz[1]=((float*)data)[2];\
			cv->xyz[2]=((float*)data)[3];\
			rv=SZ32;

	switch (poly_type)
	{
		//32b , allways in one pass :)
	case 0:	//(Non-Textured, Packed Color)
		{
			ver_32B_def;
			cv->col[0]	= (255 & (vp->vtx0.BaseCol >> 16)) / 255.f;
			cv->col[1]	= (255 & (vp->vtx0.BaseCol >> 8))  / 255.f;
			cv->col[2]	= (255 & (vp->vtx0.BaseCol >> 0))  / 255.f;
			cv->col[3]	= (255 & (vp->vtx0.BaseCol >> 24)) / 255.f;
		}
		break;
	case 1: //(Non-Textured, Floating Color)
		{
			ver_32B_def;
			cv->col[0]	= vp->vtx1.BaseR;
			cv->col[1]	= vp->vtx1.BaseG;
			cv->col[2]	= vp->vtx1.BaseB;
			cv->col[3]	= vp->vtx1.BaseA;
		}
		break;
	case 2: //(Non-Textured, Intensity)
		{
			ver_32B_def;
			cv->col[0]	= vp->vtx2.BaseInt;
			cv->col[1]	= vp->vtx2.BaseInt;
			cv->col[2]	= vp->vtx2.BaseInt;
			cv->col[3]	= vp->vtx2.BaseInt;
		}
		break;
	case 3: //(Textured, Packed Color)
		{
			ver_32B_def;
			cv->col[0]	= (255 & (vp->vtx3.BaseCol >> 16)) / 255.f;
			cv->col[1]	= (255 & (vp->vtx3.BaseCol >> 8))  / 255.f;
			cv->col[2]	= (255 & (vp->vtx3.BaseCol >> 0))  / 255.f;
			cv->col[3]	= (255 & (vp->vtx3.BaseCol >> 24)) / 255.f;

			cv->uv[0]	= vp->vtx3.u;
			cv->uv[1]	= vp->vtx3.v;
		}
		break;
	case 4://(Textured, Packed Color, 16bit UV)
		{
			ver_32B_def;
			cv->col[0]	= (255 & (vp->vtx4.BaseCol >> 16)) / 255.f;
			cv->col[1]	= (255 & (vp->vtx4.BaseCol >> 8))  / 255.f;
			cv->col[2]	= (255 & (vp->vtx4.BaseCol >> 0))  / 255.f;
			cv->col[3]	= (255 & (vp->vtx4.BaseCol >> 24)) / 255.f;
			
			cv->uv[0]	= f16(vp->vtx4.u);
			cv->uv[1]	= f16(vp->vtx4.v);
		}
		break;
	case 7://(Textured, Intensity)
		{
			ver_32B_def;
			cv->col[0]	= vp->vtx7.BaseInt;
			cv->col[1]	= vp->vtx7.BaseInt;
			cv->col[2]	= vp->vtx7.BaseInt;
			cv->col[3]	= vp->vtx7.BaseInt;

			cv->uv[0]	= vp->vtx7.u;
			cv->uv[1]	= vp->vtx7.v;
		}
		break;
	case 8://(Textured, Intensity, 16bit UV)
		{
			ver_32B_def;
			cv->col[0]	= vp->vtx8.BaseInt;
			cv->col[1]	= vp->vtx8.BaseInt;
			cv->col[2]	= vp->vtx8.BaseInt;
			cv->col[3]	= vp->vtx8.BaseInt;

			cv->uv[0]	= f16(vp->vtx8.u);
			cv->uv[1]	= f16(vp->vtx8.v);
		}
		break;
	case 9://(Non-Textured, Packed Color, with Two Volumes)
		{
			ver_32B_def;
			cv->col[0]	= (255 & (vp->vtx9.BaseCol0 >> 16)) / 255.f;
			cv->col[1]	= (255 & (vp->vtx9.BaseCol0 >> 8))  / 255.f;
			cv->col[2]	= (255 & (vp->vtx9.BaseCol0 >> 0))  / 255.f;
			cv->col[3]	= (255 & (vp->vtx9.BaseCol0 >> 24)) / 255.f;
		}
		break;
	case 10://(Non-Textured, Intensity,	with Two Volumes)
		{
			ver_32B_def;
			cv->col[0]	= vp->vtx10.BaseInt0;
			cv->col[1]	= vp->vtx10.BaseInt0;
			cv->col[2]	= vp->vtx10.BaseInt0;
			cv->col[3]	= vp->vtx10.BaseInt0;
		}
		break;
#undef ver_32B_def


#define ver_64B_def \
		if (part!=2)	/*if not on second half*/ \
				cv=verts_pv.Append();			\
			else			/*if on second half , continue input :)*/	\
			{		\
				cv=temp_lastvert;	\
				TaCmd=ta_main;		/*and return to main handling :)*/	\
			}	\
			\
			if (part==1)	/*save for second half , olny if procecing first half*/	\
			{	\
				temp_lastvert=cv;	\
			}	\
				\
			/*process first half*/	\
			if (part!=2)	\
			{	\
				rv+=SZ32;	\
				cv->xyz[0]=((float*)data)[1];	\
				cv->xyz[1]=((float*)data)[2];	\
				cv->xyz[2]=((float*)data)[3];	\
			}	\
			/*process second half*/	\
			if (part!=1)	\
			{	\
				rv+=SZ32;	\
			}
		//64b , may be on 2 pass
	case 5://(Textured, Floating Color)
		{
			ver_64B_def;
			cv->col[0]	= vp->vtx5.BaseR;
			cv->col[1]	= vp->vtx5.BaseG;
			cv->col[2]	= vp->vtx5.BaseB;
			cv->col[3]	= vp->vtx5.BaseA;

			cv->uv[0]	= vp->vtx5.u;
			cv->uv[1]	= vp->vtx5.v;
		}
		break;
	case 6://(Textured, Floating Color, 16bit UV)
		{
			ver_64B_def;
			cv->col[0]	= vp->vtx6.BaseR;
			cv->col[1]	= vp->vtx6.BaseG;
			cv->col[2]	= vp->vtx6.BaseB;
			cv->col[3]	= vp->vtx6.BaseA;

			cv->uv[0]	= f16(vp->vtx6.u);
			cv->uv[1]	= f16(vp->vtx6.v);
		}
		break;
	case 11://(Textured, Packed Color,	with Two Volumes)	
		{
			ver_64B_def;
			cv->col[0]	= (255 & (vp->vtx11.BaseCol0 >> 16)) / 255.f;
			cv->col[1]	= (255 & (vp->vtx11.BaseCol0 >> 8))  / 255.f;
			cv->col[2]	= (255 & (vp->vtx11.BaseCol0 >> 0))  / 255.f;
			cv->col[3]	= (255 & (vp->vtx11.BaseCol0 >> 24)) / 255.f;

			cv->uv[0]	= vp->vtx11.u0;
			cv->uv[1]	= vp->vtx11.v0;
		}
		break;
	case 12://(Textured, Packed Color, 16bit UV, with Two Volumes)
		{
			ver_64B_def;
			cv->col[0]	= (255 & (vp->vtx12.BaseCol0 >> 16)) / 255.f;
			cv->col[1]	= (255 & (vp->vtx12.BaseCol0 >> 8))  / 255.f;
			cv->col[2]	= (255 & (vp->vtx12.BaseCol0 >> 0))  / 255.f;
			cv->col[3]	= (255 & (vp->vtx12.BaseCol0 >> 24)) / 255.f;

			cv->uv[0]	= f16(vp->vtx12.u0);
			cv->uv[1]	= f16(vp->vtx12.v0);
		}
		break;
	case 13://(Textured, Intensity,	with Two Volumes)
		{
			ver_64B_def;
			cv->col[0]	= vp->vtx13.BaseInt0;
			cv->col[1]	= vp->vtx13.BaseInt0;
			cv->col[2]	= vp->vtx13.BaseInt0;
			cv->col[3]	= vp->vtx13.BaseInt0;

			cv->uv[0]	= vp->vtx13.u0;
			cv->uv[1]	= vp->vtx13.v0;
		}
		break;
	case 14://(Textured, Intensity, 16bit UV, with Two Volumes)
		{
			ver_64B_def;
			cv->col[0]	= vp->vtx14.BaseInt0;
			cv->col[1]	= vp->vtx14.BaseInt0;
			cv->col[2]	= vp->vtx14.BaseInt0;
			cv->col[3]	= vp->vtx14.BaseInt0;

			cv->uv[0]	= f16(vp->vtx14.u0);
			cv->uv[1]	= f16(vp->vtx14.v0);
		}
		break;
	}
#undef ver_64B_def
	if (part!=2)
	{
		f32 invW=cv->xyz[2];
		if (z_min>invW)
			z_min=invW;
		if (z_max<invW)
			z_max=invW;
		
		cv->uv[2]=1;
		cv->uv[3]=invW;

		//float z_ca=cv->xyz[2];
		//cv->xyz[2]=0;
		//z_ca*=(0.1f/z_ca);
		/*z_ca*=0.7f;
		cv->col[0]*=0.3f+z_ca;
		cv->col[1]*=0.3f+z_ca;
		cv->col[2]*=0.3f+z_ca;
		cv->col[3]*=0.3f+z_ca;*/
	}

	return rv;
};

u32 fastcall ta_mod_vol_data(Ta_Dma* data,u32 size)
{
	if (size==1)
	{
		//32B more needed , 32B done :)
		TaCmd=ta_dummy_32<ta_main>;
		return SZ32;
	}
	else
	{
		//all 64B done
		return SZ64;
	}
}
u32 fastcall ta_sprite_data(Ta_Dma* data,u32 size)
{
	if (size==1)
	{
		//32B more needed , 32B done :)
		TaCmd=ta_dummy_32<ta_main>;
		return SZ32;
	}
	else
	{
		//all 64B done
		return SZ64;
	}
}


template <u32 poly_type,u32 poly_size>
u32 fastcall ta_poly_data(Ta_Dma* dt,u32 size)
{
	u32 ci=0;
	Ta_Dma* cdp=dt;
	do 
	{
		verify(cdp->pcw.ParaType==ParamType_Vertex_Parameter);

		if (CurrentVL==0)
		{
			CurrentVL= vertlists_pv.Append();
			CurrentVL->first=verts_pv.used;
			CurrentVL->sz=0;
		}
		CurrentVL->sz++;

		if ((poly_size==SZ32) || (size>=poly_size))
		{
			ci+=poly_size;
			size-=poly_size;
			ta_handle_poly<poly_type,0>(cdp,0);
		}
		else//sz!=0 && sz<poly_size (ie , 32B part of 64B ;p)
		{
			ta_handle_poly<poly_type,1>(cdp,0);
			TaCmd=ta_handle_poly<poly_type,2>;
			ci+=SZ32;
			size-=SZ32;//0'd
		}

		if (cdp->pcw.EndOfStrip)
		{
			CurrentGP->vlc++;
			CurrentVL = 0;
			break;
		}
		cdp+=poly_size;
	}while(size!=0);

	return ci;
}
TaListFP* ta_poly_data_lut[15] = 
{
	ta_poly_data<0,SZ32>,
	ta_poly_data<1,SZ32>,
	ta_poly_data<2,SZ32>,
	ta_poly_data<3,SZ32>,
	ta_poly_data<4,SZ32>,
	ta_poly_data<5,SZ64>,
	ta_poly_data<6,SZ64>,
	ta_poly_data<7,SZ32>,
	ta_poly_data<8,SZ32>,
	ta_poly_data<9,SZ32>,
	ta_poly_data<10,SZ32>,
	ta_poly_data<11,SZ64>,
	ta_poly_data<12,SZ64>,
	ta_poly_data<13,SZ64>,
	ta_poly_data<14,SZ64>,
};

TaListFP* VerxexDataFP;
u32 fastcall poly_data_type_id(Ta_Dma* data)
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
u32 fastcall poly_header_type_size(Ta_Dma* data)
{
	if (data->pcw.Col_Type>1)
	{
		if ((data->pcw.Offset==1) || (data->pcw.Volume==1))
			return SZ64;
	}
	return SZ32;
}
//expects : Global Parameter , Control Parameter
u32 fastcall ta_main(Ta_Dma* data,u32 size)
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
					ta_list_start(data->pcw.ListType);

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

					TaGlobalParam* gp ;

					if (CurrentList==ListType_Opaque)
						gp=global_param_op_pv[id].Append();
					else
						gp=global_param_tr_pv[id].Append();

					CurrentGP=gp;

					gp->type=0;
					gp->first=vertlists_pv.used;
					gp->vlc=0;

					CurrentVL = vertlists_pv.Append();
					CurrentVL->first=verts_pv.used;
					CurrentVL->sz=0;

					if (psz>size)
					{
						//ingore next 32B (more poly data)
						TaCmd=ta_dummy_32<ta_main>;
						data+=SZ32;
						size-=SZ32;
					}
					else
					{
						//poly , 32B/64B
						VerxexDataFP=ta_poly_data_lut[id];
						data+=psz;
						size-=psz;
					}
				}
			}
			break;
			//32B
			//Sets Sprite info , and switches to ta_sprite_data function
		case ParamType_Sprite:
			{
				VerxexDataFP=ta_sprite_data;
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
//DMA from emulator :)
void TADma(u32 address,u32* data,u32 size)
{
	verify(TaCmd!=0);
	Ta_Dma* ta_data=(Ta_Dma*)data;
	while (size)
	{
		u32 sz =TaCmd(ta_data,size);
		size-=sz;
		ta_data+=sz;
	}
}

bool Ta_Init()
{
	ta_alloc_init();
	return true;
}

void Ta_Term()
{
	ta_alloc_release_all();
}

void Ta_Reset(bool manual)
{
	//*couh* right ...
}
//called when writes are made to these registers..
void Ta_SoftReset()
{
	ListIsFinished[0]=ListIsFinished[1]=ListIsFinished[2]=ListIsFinished[3]=ListIsFinished[4]=false;
	//what to do ? no lle atm sorry
}
void Ta_ListInit()
{
	//reset TA input
	TaCmd=ta_main;
	
	//Clear all lists (they are automagicaly resized, based on past 8 allocations :D)
	verts_pv.Clear();

	vertlists_pv.Clear();

	for (u32 i=0;i<6;i++)
	{
		global_param_op_pv[i].Clear();
		global_param_tr_pv[i].Clear();

		//second volumes
		//global_param_tr_pv[i].Clear();
		//global_param_tr_pv[i].Clear();
	}

	for (u32 i=6;i<14;i++)
	{
		global_param_op_pv[i].Clear();
		global_param_tr_pv[i].Clear();
	}

	ListIsFinished[0]=ListIsFinished[1]=ListIsFinished[2]=ListIsFinished[3]=ListIsFinished[4]=false;
	z_min=1000000;
	z_max=0;
	//what to do ? no lle atm sorry
}
void Ta_ListCont()
{
	ListIsFinished[0]=ListIsFinished[1]=ListIsFinished[2]=ListIsFinished[3]=ListIsFinished[4]=false;
	//what to do ? no lle atm sorry
}
