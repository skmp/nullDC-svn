using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace nullprof
{
    public partial class disasmview : UserControl
    {
        public disasmview()
        {
            InitializeComponent();
        }

        public new string Text
        {
            get
            {
                return text.Text;
            }
            set
            {
                text.Text = value;
            }
        }

        public new Font Font
        {
            get
            {
                return text.Font;
            }
            set
            {
                text.Font = value;
            }
        }
    }
}
