using System.IO;

namespace LibIR.Examples.Util
{
    public struct SoundFormat
    {
        public int SampleRate { get; set; }
        public int Channels { get; set; }
        public SoundFormat(int sr, int ch)
        {
            SampleRate = sr;
            Channels = ch;
        }
    }

    public static class WaveFile
    {
        public static float[] Read(string filename, out SoundFormat fmt)
        {
            using (FileStream fs = new FileStream(filename, FileMode.Open, FileAccess.Read))
            {
                BinaryReader reader = new BinaryReader(fs);

                // chunk 0
                int chunkID = reader.ReadInt32();
                int fileSize = reader.ReadInt32();
                int riffType = reader.ReadInt32();


                // chunk 1
                int fmtID = reader.ReadInt32();
                int fmtSize = reader.ReadInt32(); // bytes for this chunk (expect 16 or 18)

                // 16 bytes coming...
                int fmtCode = reader.ReadInt16();
                int channels = reader.ReadInt16();
                int sampleRate = reader.ReadInt32();
                int byteRate = reader.ReadInt32();
                int fmtBlockAlign = reader.ReadInt16();
                int bitDepth = reader.ReadInt16();

                if (fmtSize == 18)
                {
                    // Read any extra values
                    int fmtExtraSize = reader.ReadInt16();
                    reader.ReadBytes(fmtExtraSize);
                }

                // chunk 2
                int dataID = reader.ReadInt32();
                int bytes = reader.ReadInt32();

                // DATA!
                byte[] byteArray = reader.ReadBytes(bytes);

                int bytesForSamp = bitDepth / 8;
                int nValues = bytes / bytesForSamp;
                
                float[] asFloat;
                switch (bitDepth)
                {
                    case 64:
                        double[] asDouble = new double[nValues];
                        Buffer.BlockCopy(byteArray, 0, asDouble, 0, bytes);
                        asFloat = Array.ConvertAll(asDouble, e => (float)e);
                        break;
                    case 32:
                        asFloat = new float[nValues];
                        Buffer.BlockCopy(byteArray, 0, asFloat, 0, bytes);
                        break;
                    case 16:
                        Int16[] asInt16 = new Int16[nValues];
                        Buffer.BlockCopy(byteArray, 0, asInt16, 0, bytes);
                        asFloat = Array.ConvertAll(asInt16, e => e / (float)(Int16.MaxValue + 1));
                        break;
                    default:
                        throw new FormatException("Invalid Wave-File-Format");
                }

                fmt = new SoundFormat()
                {
                    Channels = channels,
                    SampleRate = sampleRate
                };
            
                return asFloat;
            }
        }

        private const int HEADER_SIZE = 46;

        // Saves 32-Bit audio only; Useful for testing
        public static void Write(string filename, float[] data, SoundFormat fmt)
        {
            using (FileStream fs = new FileStream(filename, FileMode.Create, FileAccess.Write))
            {
                fs.Seek(HEADER_SIZE, SeekOrigin.Begin);

                byte[] asBytes = new byte[data.Length * 4];
                Buffer.BlockCopy(data, 0, asBytes, 0, asBytes.Length);

                fs.Write(asBytes, 0, asBytes.Length);

                fs.Seek(0, SeekOrigin.Begin);
                
                byte[] riff = System.Text.Encoding.Default.GetBytes("RIFF");
                fs.Write(riff, 0, 4);

                byte[] chunkSize = BitConverter.GetBytes(fs.Length - 8);
                fs.Write(chunkSize, 0, 4);

                byte[] wave = System.Text.Encoding.Default.GetBytes("WAVE");
                fs.Write(wave, 0, 4);

                byte[] waveFmt = System.Text.Encoding.Default.GetBytes("fmt ");
                fs.Write(waveFmt, 0, 4);

                byte[] subChunk1 = BitConverter.GetBytes(18);
                fs.Write(subChunk1, 0, 4);

                UInt16 wFormat = 3; // IEEE

                byte[] audioFormat = BitConverter.GetBytes(wFormat);
                fs.Write(audioFormat, 0, 2);

                byte[] numChannels = BitConverter.GetBytes(fmt.Channels);
                fs.Write(numChannels, 0, 2);

                byte[] sampleRate = BitConverter.GetBytes(fmt.SampleRate);
                fs.Write(sampleRate, 0, 4);

                byte[] byteRate = BitConverter.GetBytes(fmt.SampleRate * fmt.Channels * 4);
                fs.Write(byteRate, 0, 4);

                UInt16 blockAlign = (ushort) (fmt.Channels * 4);
                fs.Write(BitConverter.GetBytes(blockAlign), 0, 2);

                UInt16 bps = 32;
                byte[] bitsPerSample = BitConverter.GetBytes(bps);
                fs.Write(bitsPerSample, 0, 2);

                UInt16 cbSize = 0;
                byte[] cbSizeBytes = BitConverter.GetBytes(cbSize);
                fs.Write(cbSizeBytes, 0, 2);

                byte[] datastring = System.Text.Encoding.Default.GetBytes("data");
                fs.Write(datastring, 0, 4);

                byte[] subChunk2 = BitConverter.GetBytes(asBytes.Length);
                fs.Write(subChunk2, 0, 4);
            }
        }
    }
}