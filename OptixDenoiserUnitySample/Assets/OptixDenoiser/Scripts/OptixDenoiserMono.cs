using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Experimental.Rendering;

public class OptixDenoiserMono : MonoBehaviour
{
    public Texture2D image;
    public RawImage inputImgComponent, outputImgComponent;
    byte[] data, returnData;
    OptixDenoiser optixDenoiser;

    void Start()
    {
        // Prepare the src and dst image
        RenderTexture renderTexture = new RenderTexture(image.width, image.height, 0, GraphicsFormat.R32G32B32A32_SFloat);
        renderTexture.Create();
        Graphics.Blit(image, renderTexture);
        data = GetRenderTextureData(renderTexture);
        returnData = new byte[data.Length];

        optixDenoiser = new OptixDenoiser(image.height, image.width);
        optixDenoiser.Init();
        optixDenoiser.Denoise(data, ref returnData);
        // It can be ignored if we keep denoising the image with the same size 
        optixDenoiser.Resize(960, 540);

        // Byte array to RenderTexture
        RenderTexture newRenderTexture = GetRenderTexture(returnData);

        // Display the result
        inputImgComponent.texture = renderTexture;
        outputImgComponent.texture = newRenderTexture;
    }

    byte[] GetRenderTextureData(RenderTexture m_renderTexture)
    {
        // Create a new Texture2D object and read the RenderTexture data into it
        Texture2D texture = new Texture2D(m_renderTexture.width, m_renderTexture.height, GraphicsFormat.R32G32B32A32_SFloat, TextureCreationFlags.None);
        RenderTexture.active = m_renderTexture;
        texture.ReadPixels(new Rect(0, 0, m_renderTexture.width, m_renderTexture.height), 0, 0);
        texture.Apply();
        RenderTexture.active = null;

        // Convert the Texture2D data to a byte array
        byte[] data = texture.GetRawTextureData();

        return data;
    }

    RenderTexture GetRenderTexture(byte[] m_byteArray)
    {
        // Create a new RenderTexture with the same dimensions as the byte array
        RenderTexture rt = new RenderTexture(960, 540, 0, GraphicsFormat.R32G32B32A32_SFloat);

        // Set the RenderTexture as the active render target
        RenderTexture.active = rt;

        // Create a new Texture2D and load the byte array data into it
        Texture2D texture2D = new Texture2D(960, 540, GraphicsFormat.R32G32B32A32_SFloat, TextureCreationFlags.None);
        texture2D.LoadRawTextureData(m_byteArray);
        // texture2D.LoadImage(byteArray);
        texture2D.Apply();

        // Use the Graphics API to set the texture data for the RenderTexture
        Graphics.Blit(texture2D, rt);

        // Reset the active render target
        RenderTexture.active = null;

        return rt;
    }
}
