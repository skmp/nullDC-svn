using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;
using System.Text;
using System.Drawing.Drawing2D;
using System.Drawing.Text;

namespace nullprof
{
    public delegate uint ReadMemCall(uint adr, int sz);
	/// <summary>
	/// Simple Hex view control
	/// Can dislay :DataView , ByteView , WordView , DwordView and OpcodeView
	/// </summary>
	public class HexView : System.Windows.Forms.UserControl
	{
		/// <summary>
		/// Options for Hexview crl
		/// </summary>
		public class HexDrawOptions
		{
			HexView parent=null;
			public enum DrawModeList
			{
				DataView,
				ByteView,
				WordView,
				DwordView
			}
			private DrawModeList m_DrawMode=DrawModeList.DataView;
			public DrawModeList DrawMode
			{
				get
				{
					return m_DrawMode;
				}
				set
				{
					if (m_DrawMode!=value)
					{
						m_DrawMode=value;
						parent.DrawModeChanged(m_DrawMode);
					}
				}
			}
			public HexDrawOptions(HexView par)
			{
				parent=par;
			}

			public HexDrawOptions(DrawModeList DrawMode,HexView par)
			{
				parent=par;
				this.m_DrawMode=DrawMode;
			}
		}

		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public HexView()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			// TODO: Add any initialization after the InitializeComponent call

			backbuff= new Bitmap(this.Width,this.Height);
			backbuffG = Graphics.FromImage(backbuff);
			DrawOptions=new HexDrawOptions(HexDrawOptions.DrawModeList.ByteView,this);
		}

		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Component Designer generated code
		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.vScrollBar1 = new System.Windows.Forms.VScrollBar();
			this.pictureBox1 = new System.Windows.Forms.PictureBox();
			this.SuspendLayout();
			// 
			// vScrollBar1
			// 
			this.vScrollBar1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.vScrollBar1.LargeChange = 100;
			this.vScrollBar1.Location = new System.Drawing.Point(416, 0);
			this.vScrollBar1.Maximum = 10000;
			this.vScrollBar1.Minimum = 1;
			this.vScrollBar1.Name = "vScrollBar1";
			this.vScrollBar1.Size = new System.Drawing.Size(16, 344);
			this.vScrollBar1.TabIndex = 0;
			this.vScrollBar1.Value = 5000;
			this.vScrollBar1.Scroll += new System.Windows.Forms.ScrollEventHandler(this.vScrollBar1_Scroll);
			// 
			// pictureBox1
			// 
			this.pictureBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.pictureBox1.Location = new System.Drawing.Point(0, 0);
			this.pictureBox1.Name = "pictureBox1";
			this.pictureBox1.Size = new System.Drawing.Size(408, 344);
			this.pictureBox1.TabIndex = 1;
			this.pictureBox1.TabStop = false;
			this.pictureBox1.Paint += new System.Windows.Forms.PaintEventHandler(this.pictureBox1_Paint);
			// 
			// HexView
			// 
			this.Controls.Add(this.pictureBox1);
			this.Controls.Add(this.vScrollBar1);
			this.Name = "HexView";
			this.Size = new System.Drawing.Size(432, 344);
			this.Resize += new System.EventHandler(this.HexView_Resize);
			this.Load += new System.EventHandler(this.HexView_Load);
			this.ResumeLayout(false);

		}
		#endregion

		

		public ReadMemCall ReadMemFp;

		private void HexView_Load(object sender, System.EventArgs e)
		{
		
		}
		public void DrawModeChanged(HexDrawOptions.DrawModeList dm)
		{
			RedrawBB();
			this.Refresh();
		}

		static Bitmap backbuff;
		static Graphics backbuffG;

		private System.Windows.Forms.VScrollBar vScrollBar1;//this is the draw back buffer :)
		bool backbufflocked=false;

		public HexDrawOptions DrawOptions;
		private System.Windows.Forms.PictureBox pictureBox1;
		
		private void RedrawBB()
		{
			backbufflocked=true;//not time for redraws :)
			backbuffG.Clear(this.BackColor);
			DrawHexView(backbuffG,DrawOptions,new RectangleF(0,0,this.Width,this.Height));
			backbufflocked=false;
		}

		private uint addr_hvs=0;
		public uint Address
		{
			get
			{
				return addr_hvs;
			}
			set
			{
				if (addr_hvs==value)
					return;
				addr_hvs=value;
				RedrawBB();
				this.Refresh();
			}
		}
		//hex view is organised like that :

		//Data view :
		//0xFFFF FFFF : 00 00 00 00 00 00 00 | "AAAAAAA"
		//Byte View
		//0xFFFF FFFF : 00 | "A"
		//Word View
		//0xFFFF FFFF : 00 00 | "AA"
		//Dword View
		//0xFFFF FFFF : 00 00 00 00 00 | "AAAA"
		//we are using a fixed size font , so we know the size of each char
		//this means that we need
		//generaly
		//14 * fntsz for string "0xFFFF FFFF : " ->(for all views)
		//specialy
		//
		//for b/w/dw :
		//(1+3*numchar)*fntsz for " 00 *" strings
		// 1*fntsz (for "|")
		// numchar*fntsz +3 for the " \"A*\"" strings
		//
		//for data view : 
		//things are more complex :) we muc do more calcs
		//left to be documented later (moo)
		int DataViewbpl=2;//bytes per line
		private void DrawHexView(Graphics e,HexDrawOptions opt,RectangleF ctrlSz)
		{
			
			PointF DrawPnt= new PointF(0,0);

			//hacks
			uint Addr=addr_hvs;
			//end of hacks

			float fntszy=Font.GetHeight(e.DpiY);
			float fntsz= MeasureDisplayStringWidth(e,"AAAA",this.Font)/4.0f;

			switch (opt.DrawMode)
			{
				case HexDrawOptions.DrawModeList.ByteView:
				{
					DataViewbpl=1;
					StringBuilder sb=new StringBuilder("0x");
					sb.Append(Hex(Addr>>16,4,'0'));
					sb.Append(' ');
					sb.Append(Hex(Addr&0xFFFF,4,'0'));
					sb.Append(" ");
				
					SizeF szf=new SizeF(MeasureDisplayStringWidth(e,sb.ToString(),this.Font),this.Font.GetHeight(e.DpiY));
					e.DrawLine(Pens.Black,new PointF(szf.Width,0),new PointF(szf.Width,ctrlSz.Height));
					
					szf.Width+=MeasureDisplayStringWidth(e," 00",this.Font);
					e.DrawLine(Pens.Black,new PointF(szf.Width,0),new PointF(szf.Width,ctrlSz.Height));

				}
					break;

				case HexDrawOptions.DrawModeList.DataView:
				{
					StringBuilder sb=new StringBuilder("0x");
					sb.Append(Hex(Addr>>16,4,'0'));
					sb.Append(' ');
					sb.Append(Hex(Addr&0xFFFF,4,'0'));
					sb.Append(" ");
				
					SizeF szf=new SizeF(MeasureDisplayStringWidth(e,sb.ToString(),this.Font),this.Font.GetHeight(e.DpiY));
					e.DrawLine(Pens.Black,new PointF(szf.Width,0),new PointF(szf.Width,ctrlSz.Height));

					float pixrem=(ctrlSz.Width-16)-szf.Width;
					int charrem=(int) (pixrem/fntsz);
					//char count : 4*bytes +1
					if (charrem>4)
					{
						DataViewbpl=(charrem-1)>>2;	
					}
					else
					{
						DataViewbpl=1;
					}

					sb=new StringBuilder();
					for (int i=0;i<DataViewbpl;i++)
					{
						sb.Append(" 00");							  
					}
					szf.Width+=MeasureDisplayStringWidth(e,sb.ToString(),this.Font);
					e.DrawLine(Pens.Black,new PointF(szf.Width,0),new PointF(szf.Width,ctrlSz.Height));
				}
					break;

				case HexDrawOptions.DrawModeList.DwordView:
				{
					DataViewbpl=4;
					StringBuilder sb=new StringBuilder("0x");
					sb.Append(Hex(Addr>>16,4,'0'));
					sb.Append(' ');
					sb.Append(Hex(Addr&0xFFFF,4,'0'));
					sb.Append(" ");
				
					SizeF szf=new SizeF(MeasureDisplayStringWidth(e,sb.ToString(),this.Font),this.Font.GetHeight(e.DpiY));
					e.DrawLine(Pens.Black,new PointF(szf.Width,0),new PointF(szf.Width,ctrlSz.Height));
					
					szf.Width+=MeasureDisplayStringWidth(e," 00 00 00 00",this.Font);
					e.DrawLine(Pens.Black,new PointF(szf.Width,0),new PointF(szf.Width,ctrlSz.Height));						
				}
					break;

				case HexDrawOptions.DrawModeList.WordView:
				{
					DataViewbpl=2;
					StringBuilder sb=new StringBuilder("0x");
					sb.Append(Hex(Addr>>16,4,'0'));
					sb.Append(' ');
					sb.Append(Hex(Addr&0xFFFF,4,'0'));
					sb.Append(" ");
				
					SizeF szf=new SizeF(MeasureDisplayStringWidth(e,sb.ToString(),this.Font),this.Font.GetHeight(e.DpiY));
					e.DrawLine(Pens.Black,new PointF(szf.Width,0),new PointF(szf.Width,ctrlSz.Height));
					
					szf.Width+=MeasureDisplayStringWidth(e," 00 00",this.Font);
					e.DrawLine(Pens.Black,new PointF(szf.Width,0),new PointF(szf.Width,ctrlSz.Height));
				}
					break;
			}


			//ok , i'ts party time
			while(DrawPnt.Y<ctrlSz.Height)
			{
				//DrawHeader				
				DrawString(e,"0x",this.Font,Brushes.Black,ref DrawPnt);
				
				DrawString(e,Hex(Addr,8,'0'),this.Font,Brushes.Black,ref DrawPnt);

				DrawString(e," ",this.Font,Brushes.Black,ref DrawPnt);
				//e.DrawString(sb.ToString(),this.Font,Brushes.Black,DrawPnt);
				//DrawPnt.X+=e.MeasureString(sb.ToString(),this.Font).Width;

				//draw data and string :)
				switch (opt.DrawMode)
				{
					case HexDrawOptions.DrawModeList.ByteView:
						{
							byte[] tmp=ReadDataArr(Addr,1);
							DrawByteDump_Swap(e,tmp,this.Font,Brushes.Black,ref DrawPnt);
						//	byte tmp=(byte)ReadData(Addr,1);
						//	DrawString(e," " + Hex(tmp,2,'0') + " " + ((char)tmp),this.Font,Brushes.Black,ref DrawPnt);
						}
						break;

					case HexDrawOptions.DrawModeList.DataView:
					{
						byte[] tmp=ReadDataArr(Addr,DataViewbpl);
						DrawByteDump_NoSwap(e,tmp,this.Font,Brushes.Black,ref DrawPnt);
					}
						break;

					case HexDrawOptions.DrawModeList.DwordView:
					{
						byte[] tmp=ReadDataArr(Addr,4);
						DrawByteDump_Swap(e,tmp,this.Font,Brushes.Black,ref DrawPnt);
					}	
						break;

					case HexDrawOptions.DrawModeList.WordView:
					{
						byte[] tmp=ReadDataArr(Addr,2);
						DrawByteDump_Swap(e,tmp,this.Font,Brushes.Black,ref DrawPnt);
					}
						break;
				}

				//finish up line  , reset draw pos ect :)
				DrawPnt.Y+=fntszy;
				DrawPnt.X=0;
				e.DrawLine(Pens.Black,DrawPnt,new PointF(ctrlSz.Width,DrawPnt.Y));
				switch (opt.DrawMode)
				{
					case HexDrawOptions.DrawModeList.ByteView:
						Addr+=1;
						break;

					case HexDrawOptions.DrawModeList.DataView:
						Addr+=(uint)DataViewbpl;
						break;

					case HexDrawOptions.DrawModeList.DwordView:
						Addr+=4;
						break;

					case HexDrawOptions.DrawModeList.WordView:
						Addr+=2;
						break;
				}
			}
		}

		void DrawString(Graphics e,string Text,Font fnt,Brush brsh,ref PointF DrawPnt)
		{
			e.DrawString(Text,fnt,brsh,DrawPnt);
			DrawPnt.X+=MeasureDisplayStringWidth(e,Text,this.Font);
		}

		void DrawByteDump_Swap(Graphics e,byte[]Data,Font fnt,Brush brsh,ref PointF DrawPnt)
		{
			StringBuilder sb=new StringBuilder();
			for (int i=0;i<Data.Length ;i++)
			{
				sb.Append(" " + Hex(Data[Data.Length-i-1],2,'0'));
			}

			sb.Append(" ");

			sb.Append((char)Data[0]);

			DrawString(e,sb.ToString(),fnt,brsh,ref DrawPnt);

			for (int i=1;i<Data.Length ;i++)
			{
				DrawString(e,((char)Data[i]).ToString(),fnt,brsh,ref DrawPnt);
				//sb.Append((char)Data[i]);
			}
			
		}

		void DrawByteDump_NoSwap(Graphics e,byte[]Data,Font fnt,Brush brsh,ref PointF DrawPnt)
		{
			StringBuilder sb=new StringBuilder();
			for (int i=0;i<Data.Length ;i++)
			{
				sb.Append(" " + Hex(Data[i],2,'0'));
			}

			sb.Append(" ");

			sb.Append((char)Data[0]);

			DrawString(e,sb.ToString(),fnt,brsh,ref DrawPnt);

			for (int i=1;i<Data.Length ;i++)
			{
				DrawString(e,((char)Data[i]).ToString(),fnt,brsh,ref DrawPnt);
			}
			
		}

		static public float MeasureDisplayStringWidth(Graphics graphics, string text,Font font)
		{
			System.Drawing.StringFormat format  = new System.Drawing.StringFormat ();
			System.Drawing.RectangleF   rect    = new System.Drawing.RectangleF(0, 0,1000, 1000);
			System.Drawing.CharacterRange[] ranges  = 
													{
														new System.Drawing.CharacterRange(0, 
														text.Length)
													};

			System.Drawing.Region[]         regions = new System.Drawing.Region[1];

			format.SetMeasurableCharacterRanges (ranges);

			regions = graphics.MeasureCharacterRanges (text, font, rect, format);
			rect    = regions[0].GetBounds (graphics);

			return rect.Width;
		}

		private string Hex(uint num,int numfill,char fillchar)
		{
			return Convert.ToString(num,16).ToUpper().PadLeft(numfill,fillchar);
		}
	
		private uint ReadData(uint addr,int sz)
		{
			if (ReadMemFp!=null)
				return ReadMemFp(addr,sz);
			else
				return (uint)(addr &((1<<(sz*8)))-1);
		}

		private byte[] ReadDataArr(uint addr,int sz)
		{
			byte[] t = new byte[sz];
            uint i = 0;
            //TODO : Check this Idea
            uint align = 4-( addr & 0x3)&0x3;
            if (sz >= 4)
            {
                for (; i < (align); i++)
                {
                    t[i] = (byte)ReadData(addr + i, 1);
                    sz -= 1;
                }


                //We are 4 byte alligned now!
                while (sz >= 4)
                {
                    //TODO : Check this Convertsion
                    uint data = ReadData(addr + i, 4);
                    t[i + 0] = (byte)(data >> 0);
                    t[i + 1] = (byte)(data >> 8);
                    t[i + 2] = (byte)(data >> 16);
                    t[i + 3] = (byte)(data >> 24);
                    i += 4;
                    sz -= 4;
                }

                while (sz >= 2)
                {
                    //TODO : Check this Convertsion
                    uint data = ReadData(addr + i, 2);
                    t[i + 0] = (byte)(data >> 0);
                    t[i + 1] = (byte)(data >> 8);
                    i += 2;
                    sz -= 2;
                }
            }

			for (;i<sz;i++)
			{
				t[i]=(byte)ReadData(addr+i,1);
			}
			return t;
		}

		private void HexView_Resize(object sender, EventArgs e)
		{
            //TODO : Optimise resize so we reasize bb olny when needed
			if (this.Height>0)
			{
				backbuff= new Bitmap(this.Width,this.Height);
				backbuffG = Graphics.FromImage(backbuff);
				RedrawBB();
			}
		}

		private void pictureBox1_Paint(object sender, PaintEventArgs e)
		{
			//copy back buffer to screen
			if (!backbufflocked && backbuff!=null)
			{
                e.Graphics.DrawImage(backbuff, e.ClipRectangle, e.ClipRectangle, GraphicsUnit.Pixel);
				//e.Graphics.DrawImageUnscaled(backbuff,0,0);//just copy the back buffer
			}
		}


		bool insc=false;
		private void vScrollBar1_Scroll(object sender, System.Windows.Forms.ScrollEventArgs e)
		{
			if (!insc)
			{
				insc = true;

				int offset = e.NewValue - 5000;
				offset*=DataViewbpl;
				e.NewValue = 5000;
				if (offset != 0)
				{
					Address = ((uint)(Address + (offset )));
				}
				insc = false;
			}
		}
        public nullprof.HexView.HexDrawOptions.DrawModeList DrawMode
        {
            get
            {
                return DrawOptions.DrawMode;
            }
            set
            {
                DrawOptions.DrawMode = value;
            }
        }
	}
}
