using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace nullprof
{
    public unsafe static class x86Disasm
    {
        //long __stdcall Disasm(unsigned char *data, char *output, int outbufsize, int segsize,
        //long offset, int autosync, unsigned long prefer)
        [DllImport("nDisasm.dll", EntryPoint = "_Disasm@28")]
        static extern uint Disasm_Nasm(byte* pFirst, byte* pDisasm, int outbufsize, int segsize,
            int offset, int autosync, uint prefer);

        /*static string DisasmBytes( byte[] bytes)
        {
            DisasmBytes(bytes);
        }*/
        static byte* temp_buffer = (byte*)Marshal.AllocCoTaskMem(2048);
        //static ~x86Disasm()
        //{
        //  Marshal.FreeCoTaskMem(new IntPtr(rv));
        //}
        public static uint DisasmBytes(byte[] bytes, StringBuilder sb, uint index, uint offset)
        {
            byte* rv = temp_buffer;
            uint rvc = 0;
            fixed (byte* pb = &bytes[index])
            {
                rvc = Disasm_Nasm(pb, rv, 2048, 32, (int)offset, 0, 0);
            }
            uint slen = 0;
            if (rvc != 0)
            {

                while (*rv != 0)
                {
                    sb.Append((char)*rv++);
                    slen++;
                }
            }
            else
            {
                slen = (uint)sb.Length;
                sb.Append("db 0x");
                sb.Append(Convert.ToString( bytes[index],16));
                rvc = 1;
                slen = (uint)sb.Length - slen;
            }
            uint padlen = slen > 30 ? 1 : 30 - slen;
            //padlen=padlen<1?1:padlen;
            while (padlen-- > 0)
                sb.Append(" ");

            sb.Append(";");
            sb.Append(rvc);
            if (rvc > 1)
                sb.Append(" bytes , ");
            else
                sb.Append(" byte  , ");
            for (uint i = index; i < (index + rvc); i++)
            {
                if (bytes.Length > i)
                {
                    sb.Append(Convert.ToString(bytes[i], 16));
                    sb.Append(" ");
                }
                else
                    sb.Append("...");
            }
            return rvc;
        }

        public static uint DisasmBytes(byte[] bytes, out string Disasm, uint index, uint offset)
        {
            StringBuilder sb = new StringBuilder();
            uint rvc = DisasmBytes(bytes, sb, index, offset);
            Disasm = sb.ToString();
            return rvc;
        }
        public static string DisasmBytes(uint offset, params byte[] bytes)
        {
            string rv;
            DisasmBytes(bytes, out rv, 0, offset);
            return rv;
        }

        public static string DisasmBytesBlock(uint offset, params byte[] bytes)
        {
            StringBuilder sb = new StringBuilder();
            uint index = 0;
            while (index < (bytes.Length - 1))
            {
                sb.Append("0x");
                string t = Convert.ToString(index + offset, 16);
                if (t.Length < 8)
                    sb.Append('0', 8 - t.Length);
                sb.Append(t);
                sb.Append(": ");
                uint len = DisasmBytes(bytes, sb, index, index + offset);
                index += len == 0 ? 1 : len;
                sb.Append('\n');
            }
            return sb.ToString();
        }
    }
}
