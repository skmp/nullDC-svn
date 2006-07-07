using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Collections;
using System.Net;

//Declare Function disasm_vb Lib "ndisasm_dll.dll" Alias "_disasm_vb@8" 
//(ByRef dat As Byte, ByRef strout As Byte) As Long

//good old vb code .. mah i miss it :(
/*
'Use nasm to Disassemble
Function GetAsmFromOpcodeNasm(ByVal opcode As String) As String
Dim temp(23) As Byte, sOut() As Byte, tind As Long, sout2 As String

    
    'write the data
    ReDim sOut(255)
    Do
        temp(tind) = CByte(val("&h" & GetFirstWord(opcode)))
        tind = tind + 1
        RemFisrtWord opcode: opcode = Trim$(opcode)
    Loop While Len(opcode)
    disasm_vb temp(0), sOut(0)
    ReDim Preserve sOut(Find0(sOut) - 1)
    sout2 = StrConv(sOut, vbUnicode)
    sout2 = replace0xwith0yyyh(Trim$(sout2))
    
    If Len(sout2) = 0 Then ErrorBox "Can't fix asm listing.." & vbNewLine & sout2, "modInlineAsm", "GetAsmFromOpcodeNasm"

    
    GetAsmFromOpcodeNasm = sout2
End Function
*/

namespace nullprof
{
    using u64 = UInt64;
    using u32 = UInt32;
    using u16 = UInt16;

    using s64 = Int64;
    using s32 = Int32;
    using s16 = Int16;
    using ptr = UInt32;
    using ndc = nullDCInstance;
using System.Net.Sockets;
using System.Threading;
    public partial class Form1 : Form
    {
        private ListViewItemComparer _lvwItemComparer=new ListViewItemComparer();

        //nullDCInstance ndc = new nullDCInstance();
        public Form1()
        {
            InitializeComponent();
            this.listView1.ListViewItemSorter = _lvwItemComparer;
        }
        
        private void Form1_Load(object sender, EventArgs e)
        {
            //string d1 = x86Disasm.DisasmBytes(0x40000, 0xE8, 0, 0, 0, 0, 0);
        }
        static string h32b(u32 num)
        {
            return "0x" + Convert.ToString(num, 16).PadRight(8,'0');
        }
        private void button1_Click(object sender, EventArgs e)
        {
            listView1.Items.Clear();
            Sh4Blocks.GetBlocks(BlockListType.MostTimeConsuming, 30);
            disasmview1.Text = x86Disasm.DisasmBytesBlock(
                Sh4Blocks.blocks[0].x86Code
                , Sh4Blocks.blocks[0].CompiledBlock
                );
            for (u32 i = 0; i < Sh4Blocks.blocks.Length; i++)
            {
                ListViewItem temp = new ListViewItem
                (
                    new string[]
                    {
                        h32b(Sh4Blocks.blocks[i].address),
                        Sh4Blocks.blocks[i].size.ToString(),
                        Sh4Blocks.blocks[i].Cycles.ToString(),

                        Sh4Blocks.blocks[i].Calls.ToString(),
                        Sh4Blocks.blocks[i].CallTime.ToString(),

                        Sh4Blocks.blocks[i].x86CyclesPerCall.ToString(),
                        Sh4Blocks.blocks[i].x86CyclesPerSh4Cycle.ToString(),
                        Sh4Blocks.blocks[i].x86CyclesPerSh4Op.ToString()
                    }
                );
                temp.Tag = Sh4Blocks.blocks[i];
                listView1.Items.Add(temp);
            }
        }

        // Perform Sorting on Column Headers
        private void listView1_ColumnClick(
            object sender,
            System.Windows.Forms.ColumnClickEventArgs e)
        {

            // Determine if clicked column is already the column that is being sorted.
            if (e.Column == _lvwItemComparer.SortColumn)
            {
                // Reverse the current sort direction for this column.
                if (_lvwItemComparer.Order == SortOrder.Ascending)
                {
                    _lvwItemComparer.Order = SortOrder.Descending;
                }
                else
                {
                    _lvwItemComparer.Order = SortOrder.Ascending;
                }
            }
            else
            {
                // Set the column number that is to be sorted; default to ascending.
                _lvwItemComparer.SortColumn = e.Column;
                _lvwItemComparer.Order = SortOrder.Ascending;
            }

            
            // Perform the sort with these new sort options.
            try
            {
                this.listView1.Sort();
            }catch
            {}
        }

        private void listView1_SelectedIndexChanged(object sender, EventArgs e)
        {
            try
            {
                Sh4Block blk = (Sh4Block)listView1.SelectedItems[0].Tag;
                disasmview1.Text = x86Disasm.DisasmBytesBlock(blk.x86Code, blk.CompiledBlock);
            }
            catch { }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            ndc.SendString("pclear");
            ndc.dpa t = new nullDCInstance.dpa();
            ndc.GetPacket(ref t);
        }


    }

    // This class is an implementation of the 'IComparer' interface.
    public class ListViewItemComparer : IComparer
    {
        // Specifies the column to be sorted
        private int ColumnToSort;

        // Specifies the order in which to sort (i.e. 'Ascending').
        private SortOrder OrderOfSort;

        // Case insensitive comparer object
        private CaseInsensitiveComparer ObjectCompare;

        // Class constructor, initializes various elements
        public ListViewItemComparer()
        {
            // Initialize the column to '0'
            ColumnToSort = 0;

            // Initialize the sort order to 'none'
            OrderOfSort = SortOrder.None;

            // Initialize the CaseInsensitiveComparer object
            ObjectCompare = new CaseInsensitiveComparer();
        }

        // This method is inherited from the IComparer interface.
        // It compares the two objects passed using a case
        // insensitive comparison.
        //
        // x: First object to be compared
        // y: Second object to be compared
        //
        // The result of the comparison. "0" if equal,
        // negative if 'x' is less than 'y' and
        // positive if 'x' is greater than 'y'
        public int Compare(object x, object y)
        {
            int compareResult;
            ListViewItem listviewX, listviewY;

            // Cast the objects to be compared to ListViewItem objects
            listviewX = (ListViewItem)x;
            listviewY = (ListViewItem)y;

            // Case insensitive Compare
            if (SortColumn == 0)
            {
                compareResult = ObjectCompare.Compare(
                    listviewX.SubItems[ColumnToSort].Text,
                    listviewY.SubItems[ColumnToSort].Text
                );
            }
            else
            {
                compareResult = double.Parse(listviewX.SubItems[ColumnToSort].Text).CompareTo(
                    double.Parse(
                    listviewY.SubItems[ColumnToSort].Text)
                    );
            }

            // Calculate correct return value based on object comparison
            if (OrderOfSort == SortOrder.Ascending)
            {
                // Ascending sort is selected, return normal result of compare operation
                return compareResult;
            }
            else if (OrderOfSort == SortOrder.Descending)
            {
                // Descending sort is selected, return negative result of compare operation
                return (-compareResult);
            }
            else
            {
                // Return '0' to indicate they are equal
                return 0;
            }
        }

        // Gets or sets the number of the column to which to
        // apply the sorting operation (Defaults to '0').
        public int SortColumn
        {
            set
            {
                ColumnToSort = value;
            }
            get
            {
                return ColumnToSort;
            }
        }

        // Gets or sets the order of sorting to apply
        // (for example, 'Ascending' or 'Descending').
        public SortOrder Order
        {
            set
            {
                OrderOfSort = value;
            }
            get
            {
                return OrderOfSort;
            }
        }
    }


    public abstract class IndcUpdatable
    {
        static u32 uuid_c = 0;
        u32 uuid_t = uuid_c++;

        public u32 uidc
        {
            get { return uuid_t; }
        }

        public abstract void killme();
        public abstract void killme_2();
    }
    //block info :
    //address
    //size
    //Cycles
    //x86Code
    //x86CodeSize
    //Calls
    //CallTime
    public class Sh4Block
    {
        public u32 address;
        public u32 size;
        public u32 Cycles;
        public ptr sh4Code;
        public ptr x86Code;
        public u32 x86CodeSize;
        public u32 Calls;
        public u64 CallTime;

        public double x86CyclesPerCall
        {
            get
            {
                return (double)CallTime / (double)Calls;
            }
        }

        public double x86CyclesPerSh4Op
        {
            get
            {
                return x86CyclesPerCall / (double)(size>>1);
            }
        }

        public double x86CyclesPerSh4Cycle
        {
            get
            {
                return x86CyclesPerCall / (double)Cycles;
            }
        }

        byte[] CompiledBlock_Cache=null;
        public byte[] CompiledBlock
        {
            get
            {
                if (CompiledBlock_Cache == null)
                {
                    ndc.SendString("memget " + Convert.ToString(x86Code, 16) + " " + x86CodeSize.ToString());
                    nullDCInstance.dpa packet= new nullDCInstance.dpa();
                    ndc.GetPacket(ref packet);
                    CompiledBlock_Cache = packet.data;
                }
                return CompiledBlock_Cache;
            }
        }

        byte[] Sh4BlockBytes_Cache = null;
        public byte[] Sh4BlockBytes
        {
            get
            {
                if (Sh4BlockBytes_Cache == null)
                {
                    ndc.SendString("memget " + Convert.ToString(sh4Code, 16) + " " + size.ToString());
                    nullDCInstance.dpa packet = new nullDCInstance.dpa();
                    ndc.GetPacket(ref packet);
                    Sh4BlockBytes_Cache = packet.data;
                }
                return Sh4BlockBytes_Cache;
            }
        }


   

        public void ReadFromPacket(ref nullprof.nullDCInstance.dpa packet)
        {
           // u32 addr;
           address = packet.ReadU32();
           // void* sh4_code;
           sh4Code = packet.ReadU32();
           // void* x86_code;
           x86Code = packet.ReadU32();
           // u32 sh4_bytes;
           size = packet.ReadU32();
           // u32 x86_bytes;
           x86CodeSize = packet.ReadU32();
           // u32 sh4_cycles;
           Cycles = packet.ReadU32();
            //u64 time;
           CallTime = packet.ReadU64();
           // u32 calls;
           Calls = packet.ReadU32();
        }
    }

    public enum BlockListType
    {
        //      #define ALL_BLOCKS 0
        //#define PCALL_BLOCKS 1
        //#define PTIME_BLOCKS 2
        //#define PSLOW_BLOCKS 3

        All=0,
        MostCalled=1,
        WorstRatio=2,
        MostTimeConsuming=3
    }
    public static class Sh4Blocks
    {
        public static Sh4Block[] blocks;
        public static void GetBlocks(BlockListType type,u32 count)
        {
            nullDCInstance.dpa packet = new nullDCInstance.dpa();
            ndc.SendString("blocks " + ((u32)type).ToString() + " " + count.ToString() + " 0");//the last 0 is for binary transfer :)
            ndc.GetPacket(ref packet);
            u32 blk_num = (u32)(packet.sz / (9 * 4));
            blocks = new Sh4Block[blk_num];
            for (u32 i = 0; i < blk_num; i++)
            {
                blocks[i] = new Sh4Block();
                blocks[i].ReadFromPacket(ref packet);
            }
        }
    }
    //nullprof  rcp
    //command
    //"ver"    -> get ndc version string
    //"blocks [all|pcall|ptime|pslow] {##}" : get block list , all gets all blocks (## is ignored)
    // pcall/time/slow -> get most called/most time consuming/slower (sh4/x86 cycles)
    //"pclear" -> clear profiler data
    //"blockinfo [id|addr] ##" -> get block info for block id=## or addr=##
    //"memget ##1,##2"           -> get memory data dump (duh realy ?) , starting at addr=##1 for ##2 bytes , ##1 has to be formated as bytes
    //"memset ##1,##2"           -> set memory data , addr=##1 size=##2. size must match data block sent exactly after this packet

    //end :p
    //data packet handling
    //u32 size;
    //u8[] data;
    //text is trasnfered at 8 bit ansi text
    //each packet is decoded as command , unless were waiting a data packet
    public static class nullDCInstance
    {

        static nullDCInstance()
        {
            //th.Start();
            soc.Connect("192.168.1.33", 1337);
        }
        
        public unsafe struct dpa
        {
            public s32 sz;
            public byte[] data;

            public u32 read_index;
            public u32 ReadU32()
            {
                fixed (byte* bptr = &data[read_index])
                {
                    read_index += 4;
                    return *(u32*)bptr;
                }
            }
            public u64 ReadU64()
            {
                fixed (byte* bptr = &data[read_index])
                {
                    read_index += 8;
                    return *(u64*)bptr;
                }
            }

            public ptr ReadPtr()
            {
                return (ptr)ReadU32();
            }
        }
        
        public static void GetPacket(ref dpa packet)
        {
            byte[] temp = new byte[4];
            soc.Receive(temp, 0, 4, SocketFlags.None);
            s32 len = (temp[0] | (temp[1]<<8)| (temp[2]<<16) | (temp[3]<<24));
            temp = new byte[len];
            int idx = 0;
            packet.sz = len;

            while (len != 0)
            {
                int sz=soc.Receive(temp, idx, len, SocketFlags.None);
                idx += sz;
                len -= sz;

            }
            packet.data = temp;
        }
        public static void SendString( string teh_str)
        {
            dpa packet;
            packet.sz = teh_str.Length;
            char[] shit = teh_str.ToCharArray();

            packet.data = new byte[shit.Length];
            packet.read_index = 0;
            for (int i = 0; i < shit.Length; i++)
                packet.data[i] = (byte)shit[i];
            SendPacket(ref packet);
        }
        public static void ProcPacket(ref dpa packet)
        {
            for (int i = 0; i < packet.sz; i++)
                Console.Write((char)packet.data[i]);
            Console.Write('\n');
        }
        public static void SendPacket(ref dpa packet)
        {
            byte[] temp = new byte[4];
            // packet.sz 
            temp[0]= (byte)(packet.sz & 0xFF);
            temp[1] = (byte)((packet.sz << 8) & 0xFF);
            temp[2] = (byte)((packet.sz << 16) & 0xFF);
            temp[3] = (byte)((packet.sz << 24) & 0xFF);

            soc.Send(temp, 0, 4, SocketFlags.None);
            
            soc.Send(packet.data, 0, packet.sz, SocketFlags.None);
        }
        static Socket soc = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        /*static Thread th = new Thread(TcpThead);
        static void TcpThead()
        {
            soc.Connect("127.0.0.1", 1337);
            dpa packet=new dpa();
            while (true)
            {
                GetPacket(soc, ref packet);
                ProcPacket(soc, ref packet);
            }
        }*/
        
        static Random rand = new Random();
        public static byte[] GetBlock(uint sh4_address)
        {
            byte[] temp = new byte[2048];
            rand.NextBytes(temp);
            return temp;
        }
    }

}