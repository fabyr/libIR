using System;
using LibIR;
using LibIR.Examples.Util;

namespace LibIR.Examples
{
    public class Program
    {
        public static void Main(string[] args)
        {
            const string irFile = "/home/fabyr/Desktop/ir1.wav";
            const string signalFile = "/home/fabyr/Desktop/sig1.wav";
            const string outputFile = "/home/fabyr/Desktop/out1.wav";

            SoundFormat irSf, sigSf;
            float[] ir = WaveFile.Read(irFile, out irSf);
            float[] signal = WaveFile.Read(signalFile, out sigSf);

            // Make sure ir and signal have the same SoundFormat (same sample rate and mono)
            // Code to check for that is not here

            Convolver c = new Convolver(ir, new[] { 1024, 1024, 2048, 4096, 8192, 16384 });
            float[] s1 = new float[1024];
            float[] o1;
            float[] output = new float[signal.Length + ir.Length - 1];

            // This is not quite correct (a bit will be missing at the end) 
            // but good enough for demonstration purposes
            for(int i = 0; i < output.Length - 1024; i += 1024)
            {
                if(i + 1024 >= signal.Length)
                    Array.Clear(s1);
                else
                    Array.Copy(signal, i, s1, 0, s1.Length);
                o1 = c.ComputeBlock(s1);
                Array.Copy(o1, 0, output, i, o1.Length);
            }
            WaveFile.Write(outputFile, output, sigSf);
            Console.WriteLine("Done!");
        }
    }
}