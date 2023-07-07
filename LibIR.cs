using System;
using System.Runtime.InteropServices;

namespace LibIR
{
    public struct Complex
    {
        public float X, Y;
    }

    public class Convolver
    {
        private Complex[] ir;
        private Complex[]? irFft = null;

        private ConvolveData _data;

        private int[] schedule;

        private int BaseBlockSize => schedule[0];

        public Convolver(float[] impulseResponse, int[] schedule, IRFlags flags = IRFlags.IR_FFTW_FLAG_MEASURE)
        {
            if(schedule.Length == 0)
                throw new ArgumentException("schedule is empty", "schedule");
            if(!ValidateSchedule(schedule))
                throw new ArgumentException("invalid schedule", "schedule");
            this.schedule = schedule;

            ir = new Complex[impulseResponse.Length];
            for(int i = 0; i < ir.Length; i++)
                ir[i].X = impulseResponse[i];

            GCHandle scheduleHandle = GCHandle.Alloc(schedule, GCHandleType.Pinned);
            ConvolveSchedule structSchedule = new ConvolveSchedule()
            {
                block_sizes = scheduleHandle.AddrOfPinnedObject(),
                entries = schedule.Length
            };
            _data = LibIRCore.CreateConvolveData(structSchedule, impulseResponse.Length, flags);
            scheduleHandle.Free();

            if(!flags.HasFlag(IRFlags.IR_ALLOCATE_FFT_BUFFER))
            {
                PreprocessFft();
            }
        }

        public static bool ValidateSchedule(int[] schedule)
        {
            if(schedule.Length == 0)
                return false;
            if(schedule.Length == 1)
                return true;
            int baseBlockSize = schedule[0];
            bool valid = true;
            for(int i = 1; i < schedule.Length; i++)
                valid &= schedule[i] == baseBlockSize * (1 << (i - 1));
            return valid;
        }

        private void PreprocessFft()
        {
            irFft = new Complex[_data.fftbuffer_size];
            GCHandle irHandle = GCHandle.Alloc(ir, GCHandleType.Pinned);
            GCHandle irFftHandle = GCHandle.Alloc(irFft, GCHandleType.Pinned);

            LibIRCore.IrFft(ref _data, irHandle.AddrOfPinnedObject(), irFftHandle.AddrOfPinnedObject());

            irFftHandle.Free();
            irHandle.Free();
        }

        public float[] ComputeBlock(float[] signal)
        {
            if(signal.Length != BaseBlockSize)
                throw new ArgumentException($"Signal Length does not match the base block size ({BaseBlockSize})", "signal");
            
            Complex[] complexSignal = new Complex[signal.Length];
            Complex[] complexOut = new Complex[signal.Length];
            for(int i = 0; i < signal.Length; i++)
                complexSignal[i].X = signal[i];
            
            GCHandle signalHandle = GCHandle.Alloc(complexSignal, GCHandleType.Pinned);
            GCHandle outHandle = GCHandle.Alloc(complexOut, GCHandleType.Pinned);

            if(irFft == null)
            {
                GCHandle irHandle = GCHandle.Alloc(ir, GCHandleType.Pinned);
                LibIRCore.BlockConvolveFft(ref _data, irHandle.AddrOfPinnedObject(), signalHandle.AddrOfPinnedObject(), outHandle.AddrOfPinnedObject());
                irHandle.Free();
            }
            else
            {
                GCHandle irFftHandle = GCHandle.Alloc(irFft, GCHandleType.Pinned);
                LibIRCore.BlockConvolve(ref _data, irFftHandle.AddrOfPinnedObject(), signalHandle.AddrOfPinnedObject(), outHandle.AddrOfPinnedObject());
                irFftHandle.Free();
            }

            outHandle.Free();
            signalHandle.Free();

            float[] @out = new float[signal.Length];
            for(int i = 0; i < signal.Length; i++)
                @out[i] = complexOut[i].X;
            return @out;
        }
    }

    public enum IRFlags : int
    {   
        IR_NO_FLAGS            = 0b00000000,
        IR_ALLOCATE_FFT_BUFFER = 0b00000001,
        IR_FFTW_FLAG_MEASURE   = 0b00000010
    }

    public static class LibIRCore
    {
        public const string LibraryName = "IR";
        
        [DllImport(LibraryName)]
        public static extern ConvolveData CreateConvolveData(ConvolveSchedule schedule, int ir_samples, IRFlags flags);

        [DllImport(LibraryName)]
        public static extern void FreeConvolveData(ref ConvolveData data);

        [DllImport(LibraryName)]
        public static extern void BlockConvolve(ref ConvolveData data, IntPtr ir_fft, IntPtr sig, IntPtr @out);

        [DllImport(LibraryName)]
        public static extern void BlockConvolveFft(ref ConvolveData data, IntPtr ir, IntPtr sig, IntPtr @out);

        [DllImport(LibraryName)]
        public static extern void IrFft(ref ConvolveData data, IntPtr ir, IntPtr ir_fft);
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ConvolveSchedule
    {
        public IntPtr block_sizes;
        public int entries;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ConvolveFftSchedule
    {
        public IntPtr block_size_array, block_size_sum_array, block_size_log2_array,
                      fft_n_array, fft_n_sum_array, fft_n_log2_array, layer_array,
                      layer_size_array, layer_sum_array, layer_nblocks_array,
                      forward_plans, backward_plans;
        public int entries;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ConvolveData
    {
        public ConvolveFftSchedule schedule;
        public int in_block_size, in_block_size_log2, in_fft_size,
                   in_fft_size_log2, max_block_size, max_block_size_log2,
                   max_fft_n, max_fft_n_log2, n_blocks_ir, n_blocks_in_ir, 
                   blockbuffer_size, fftbuffer_size, x_fdl_layers;
        public IntPtr x_tdl, x_fdl, fft_buffer_in, fft_buffer_out, y_buffer, ir_buffer_fft;
        public int flags, x_fdl_at, y_dfl_at;
    }
}