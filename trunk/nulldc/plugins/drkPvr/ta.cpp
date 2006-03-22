#include "ta.h"
//Tile Accelerator state machine
#include "ta_alloc.h"

void Ta64bSecond(Ta_Dma* data);

//
ListTypes CurrentList=ListTypes::None;

void TaSpriteData(Ta_Dma* param);
void TaNormal(Ta_Dma* param);
void TaListData(Ta_Dma* param);
void TaPolyData(Ta_Dma* param);
void TaModData(Ta_Dma* param);

TaListFP* TaCmd=TaNormal;
InterruptID ListEndInterrupt[5]=
{
	InterruptID::holly_OPAQUE,
	InterruptID::holly_OPAQUEMOD,
	InterruptID::holly_TRANS,
	InterruptID::holly_TRANSMOD,
	InterruptID::holly_PUNCHTHRU
};

bool ListIsFinished[5]={false,false,false,false,false};

bool cmd_64b_pending=false;
Ta_Dma cmd_64b_data[2];
bool Is64byte(Ta_Dma* command)
{
	//poly/modvol
	if (command->pcw.ParaType==ParamType::Polygon_or_Modifier_Volume)
	{
		ListTypes cl;

		if (CurrentList==ListTypes::None)
			cl=command->pcw.ListType;	//first entry on this list ...
		else
			cl=CurrentList;

		//Modifier Volume	-- these are allways 64 bytes ...
		if ((cl==ListTypes::Opaque_Modifier_Volume) || (cl==ListTypes::Translucent_Modifier_Volume))
		{
			return true;
		}

		//normal poly (opq,trq,pt)

		//Polygon Type 2(Intensity, use Offset Color)
		if ((command->pcw.Offset) && (command->pcw.Col_Type&2))
			return true;

		//Polygon Type 4(Intensity, with Two Volumes)
		if ((command->pcw.Shadow) && (command->pcw.Col_Type&2))
			return true;

		//Polygon Type 5(Floating Color)	
		//Polygon Type 6(Floating Color, 16bit UV)
		if (command->pcw.Col_Type==1)
			return true;

		//Polygon Type 11(Textured, Packed Color,with Two Volumes)	
		//Polygon Type 12(Textured, Packed Color, 16bit UV,	with Two Volumes)
		if (command->pcw.Texture && command->pcw.Shadow && command->pcw.Col_Type==0)
			return true;
	}

	//Sprite
	if ((command->pcw.ParaType==ParamType::Sprite))
		return true;

	//vertex
	if ((command->pcw.ParaType==ParamType::Vertex_Parameter))
	{
		return false;//needs to be done
	}

	//well , it's 32b command :)
	return false;
}

//allways 32 byte writes
void TaWrite(u32* data)
{
	//for 64 b commands
	if (!cmd_64b_pending)
	{
		if (Is64byte((Ta_Dma*)data))
		{
			cmd_64b_pending=true;
			cmd_64b_data[0]=*(Ta_Dma*)data;
			return;
		}
	}
	else
	{
		cmd_64b_data[1]=*(Ta_Dma*)data;
		data=(u32*)&cmd_64b_data[0];
	}


	if (TaCmd==0)
	{
		//error
	}
	else
		TaCmd((Ta_Dma*)data);
}

/*
//no list has been inited - list has ended
void TaListInit(Ta_Dma* data)
{
	if (CurrentList!=ListTypes::None)
	{
		//error
	}
	switch(data->pcw.ParaType)
	{
	case ParamType::End_Of_List:
	case ParamType::Reserved_1:
	case ParamType::Reserved_2:
	case ParamType::Vertex_Parameter:
	case ParamType::Sprite:
	case ParamType::Object_List_Set:
	case ParamType::User_Tile_Clip:
		//Todo :invalid - add warning
		break;

	case ParamType::Polygon_or_Modifier_Volume:
		switch(data->pcw.ListType)
		{
		case ListTypes::Opaque:
		case ListTypes::Opaque_Modifier_Volume:
		case ListTypes::Translucent:
		case ListTypes::Translucent_Modifier_Volume:
		case ListTypes::Punch_Through:
			//Get the list data
			if (ListIsFinished[data->pcw.ListType])
			{
				//this is bad , must never happen ...
			}
			CurrentList=data->pcw.ListType;
			TaCmd=TaInputListData;
			break;

		default:
			//Todo :invalid - add warning
			break;
		}
		break;
	}
}

//Waiting for second half of an 64b command
TaListFP* Ta64bSecond_previus;
void Ta64bSecond(Ta_Dma* data)
{
	TaCmd=Ta64bSecond_previus;
	//call a callback for processing
}

//Waiting list data
void TaInputListData(Ta_Dma* data)
{
	switch(data->pcw.ParaType)
	{
	case ParamType::Reserved_1:
	case ParamType::Reserved_2:
	case ParamType::Polygon_or_Modifier_Volume:
		//Todo :invalid - add warning
		break;

	case ParamType::Object_List_Set:
		//Todo :not implemented - warn, needs lle
		break;

	case ParamType::User_Tile_Clip:
		//Todo :handle it
		break;

	case ParamType::Vertex_Parameter:
		//vertex param :)
		if (Is64byte(data))
		{
			Ta64bSecond_previus=TaCmd;
			TaCmd=Ta64bSecond;
		}
		break;

	case ParamType::Sprite:
		//Sprite param :)
		if (Is64byte(data))
		{
			Ta64bSecond_previus=TaCmd;
			TaCmd=Ta64bSecond;
		}
		break;

	case ParamType::End_Of_List:
		//End of list param
		//data->pcw.ListType is valid OLNY when starting a list
		RaiseInterrupt(ListEndInterrupt[CurrentList]);
		ListIsFinished[CurrentList]=true;
		//wait for next list :)
		TaCmd=TaListInit;
		break;
	}
}
*/

void SetUserClip(Ta_Dma* param)
{
	//TODO : Add support for user cliping
}
//called from sh4 context , in case of dma or SQ to TA memory , size is 32 byte transfer counts

//Okie ..
//State machine logic
//Initial state (after SoftReset) or After end of list
//TaNormal
//Can accept :
//Polygon/Mod_Volume	(depends on list type)	[Sets state to TaListData , redirects proccesing]
//Sprite										[Sets state to TaListData , redirects proccesing]
//Object_List_Set								[Sets state to TaListData , redirects proccesing]
//User_Tile_Clip								[Does not change state , Sets user clip]
void TaNormal(Ta_Dma* param)
{
	switch(param->pcw.ParaType)
	{
		//seems like a list is started ;)
		case ParamType::Polygon_or_Modifier_Volume:
		case ParamType::Sprite:
		case ParamType::Object_List_Set:
			//TODO : check for closed list
			CurrentList=param->pcw.ListType;
			TaCmd=TaListData;
			TaListData(param);//redirect proceccing
			break;

		case ParamType::User_Tile_Clip:
			SetUserClip(param);
			break;

		default:
			//warn
			printf("TaNormal : Unsuported ParaType :%d\n",param->pcw.ParaType);
			break;
	}
}

//After input for a list has been started. The first parameter is passed too.
//TaListData
//Can accept:
//(Global Params)
//Polygon/Mod_Volume	(depends on list type)	[Sets state to TaPolyOrModData]
//Sprite										[Sets state to TaSpriteData , 64b command]
//(Control Params)
//End_Of_List									[Ends curent list type , Raises interrupt , Sets state to TaNormal]
//User_Tile_Clip								[Does not change state , Sets user clip]
//Object_List_Set								[Does not change state , ignored for now (lle missing..)]
void TaListData(Ta_Dma* param)
{
	switch(param->pcw.ParaType)
	{
		case ParamType::Polygon_or_Modifier_Volume:
			if ((CurrentList==ListTypes::Opaque) || (CurrentList==ListTypes::Translucent) || (CurrentList==ListTypes::Punch_Through)) 
			{
				TaCmd=TaPolyData;
				//TODO : fix rest
			}
			else if ((CurrentList==ListTypes::Opaque_Modifier_Volume) || (CurrentList==ListTypes::Translucent_Modifier_Volume))
			{
				TaCmd=TaModData;
				//TODO : fix rest
			}
			break;

		case ParamType::Sprite:
			TaCmd=TaSpriteData;
			//TODO : fix rest
			break;

		case ParamType::End_Of_List:
			ListIsFinished[CurrentList]=true;
			RaiseInterrupt(ListEndInterrupt[CurrentList]);

			CurrentList=ListTypes::None;
			TaCmd=TaNormal;
			break;

		case ParamType::User_Tile_Clip:
			SetUserClip(param);
			break;

		default:
			//warn
			printf("TaListData : Unsuported ParaType :%d\n",param->pcw.ParaType);
			break;
	}
}

//After a Poly input has started
//TaPolyData
//Can accept:
//(Poly or mod params)
//Vertex										[will switch to TaListData on strip end]
void TaPolyData(Ta_Dma* param)
{
}
//After a Mod input has started
//TaModData
//Can accept:
//(Poly or mod params)
//Vertex										[will switch to TaListData on strip end]
void TaModData(Ta_Dma* param)
{
}

//After a Sprite input has started
//TaSpriteData
//Can accept:
//(Poly or mod params)
//Vertex										[will switch to TaListData on strip end]
void TaSpriteData(Ta_Dma* param)
{
}


//Rest shit
void TADma(u32 address,u32* data,u32 size)
{
	while(size--)
	{
		TaWrite(data);
		data+=32;
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
	//what to do ? no lle atm sorry
}
void Ta_ListCont()
{
	ListIsFinished[0]=ListIsFinished[1]=ListIsFinished[2]=ListIsFinished[3]=ListIsFinished[4]=false;
	//what to do ? no lle atm sorry
}
