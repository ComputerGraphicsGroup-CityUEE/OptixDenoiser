using System.Runtime.InteropServices;

public class OptixDenoiser
{
    #region NativePlugin
    [DllImport("OptixDenoiser", EntryPoint = "qq_init")]
    private static extern void qq_init( int width0, int height0 );
    [DllImport("OptixDenoiser", EntryPoint = "qq_denoise")]
    private unsafe static extern void qq_denoise(byte[] src, byte[] des);
    [DllImport("OptixDenoiser", EntryPoint = "qq_resize")]
    private static extern void qq_resize( int width0, int height0 );

    #endregion

    private int height;
    private int width;

    public OptixDenoiser(int m_width, int m_height)
    {
        height = m_height; 
        width = m_width;
    }

    public void Init()
    {
        qq_init(height, width);
    }

    public void Denoise(byte[] src, ref byte[] dst)
    {
        qq_denoise(src, dst);
    }

    public void Resize(int dstWidth, int dstHeight)
    {
        qq_resize(dstHeight, dstWidth);
    }
}
