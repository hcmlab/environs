package environs;
/**
 * Android Java Encoder for images and h264 streams
 * ------------------------------------------------------------------
 * Copyright (c) 2015 Chi-Tai Dang, University of Augsburg
 *
 * @author	Chi-Tai Dang, dang@hcm-lab.de
 * @version	1.0
 *
 * This file is part of the Environs framework developed at the
 * Lab for Human Centered Multimedia of the University of Augsburg.
 * http://hcm-lab.de/environs
 *
 * Environ is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * The Environs framework and this file is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Environs is also available with limitations under a different
 * license for usage in proprietary projects. For more information,
 * contact us at dang@hcm-lab.de or info@hcm-lab.de
 * --------------------------------------------------------------------
 */
import android.annotation.SuppressLint;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.view.Surface;
import java.nio.ByteBuffer;


@SuppressWarnings("deprecation")
@SuppressLint("NewApi")
public abstract class Codec {
    private static final String className = "Codec. . . . . . . . . .";

    final static String mimeType = "video/avc";

    int codecType = PortalStreamType.Video;

    public boolean useInputSurface = false;

    boolean isEncoder = false;

    MediaCodec CoDec = null;
    MediaFormat format = null;
    int frameIndex = 0;
    boolean isEoS = false;

    MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
    boolean performOutput = true;

    boolean initialized = false;
    boolean	API21 = false;

    long timeoutUs = 1000000; // -1
    ByteBuffer inputBuffers[] = null;
    ByteBuffer outputBuffers[] = null;
    boolean started = false;

    Surface surface = null;
    int width = -1;
    int height = -1;
    int frameRate = 30;
    int bitRate = 5000000;

    Codec() {
        Utils.Log1 ( className, "Construct..." );
        API21 = (android.os.Build.VERSION.SDK_INT > 20);
    }


    abstract MediaCodec getCoDec () throws Exception;

    abstract MediaFormat configureFormat (MediaFormat format) throws Exception;

    abstract boolean initCoDec ();

    abstract boolean onPreStart (MediaCodec codec);

    public void setFormat (MediaFormat format)
    {
        this.format = format;
        width = format.getInteger(MediaFormat.KEY_WIDTH);
        height = format.getInteger(MediaFormat.KEY_HEIGHT);
        bitRate = format.getInteger(MediaFormat.KEY_BIT_RATE);
        frameRate = format.getInteger(MediaFormat.KEY_FRAME_RATE);
    }

    public boolean init()
    {
        String codecType = isEncoder ? "encoder" : "decoder";

        Utils.Log1 ( className, "Init: " + codecType );

        if (initCoDec())
            return true;

        frameIndex = 0;

        if (!isEncoder && surface == null) {
            Utils.LogW(className, "Init: Invalid surface");
            return false;
        }

        if (width <= 0 || height <= 0) {
            Utils.LogW ( className, "Init: Invalid width/height" );
            return false;
        }

        if (CoDec != null) {
            Utils.LogE(className, "Init: A " + codecType + " has been created before!");
            release ( );
        }

        if (Utils.isDebug) Utils.Log ( 5, className, "Init: Creating " + codecType + " for " + mimeType );
        MediaCodec codec = null;

        if ( !Utils.isNewApi() ) {
            Utils.LogE ( className, "Init: Platform does not support " + mimeType + "!");
            return false;
        }

        try {
            codec = getCoDec();
            if (codec == null) {
                Utils.LogE ( className, "Init: Failed to create " + codecType + " for " + mimeType);
                return false;
            }

            if (Utils.isDebug) Utils.Log ( 5, className, "Init: Creating video format for given width " + width + " and height " + height );

            if ( format == null ) {
                format = MediaFormat.createVideoFormat(mimeType, width, height);
                if (format == null) {
                    Utils.LogE ( className, "Init: Video format [" + width + "/" + height + "] not supported!");
                    return false;
                }

                format = configureFormat(format);
            }


            if (Utils.isDebug) Utils.Log ( 5, className, "Init: Configuring  " + codecType + " with format and surface" );
            codec.configure(format, surface, null, isEncoder ? MediaCodec.CONFIGURE_FLAG_ENCODE : 0);

            if ( Utils.APILevel >= 18 ) {
                try {
                    MediaCodecInfo info = codec.getCodecInfo();
                    if (info == null) {
                        Utils.LogE ( className, "Init: Failed to query decoder info!");
                        return false;
                    }
                }
                catch (Exception ex) {
                    Utils.LogE ( className, "Init: [" + ex.getMessage() + "]" );
                }
            }

            if (!onPreStart(codec)) {
                return false;
            }

            if (Utils.isDebug) Utils.Log ( 5, className, "Init: Starting  " + codecType );
            codec.start();

        } catch (Exception e) {
            Utils.LogE(className, "Init: Failed to create/configure " + codecType + " with given format and surface");
            e.printStackTrace();
            return false;
        }

        if ( !API21 ) {
            if (Utils.isDebug) Utils.Log ( 5, className, "Init: API " + android.os.Build.VERSION.SDK_INT );

            if (Utils.isDebug) Utils.Log ( 5, className, "Init: Requesting input buffers" );
            inputBuffers = codec.getInputBuffers ();

            if (Utils.isDebug) Utils.Log ( 5, className, "Init: Requesting output buffers" );
            outputBuffers = codec.getOutputBuffers ();
        }

        CoDec = codec;
        initialized = true;

        if (Utils.isDebug) Utils.Log ( 5, className, "Init: " + codecType + " successfully initialized" );
        return true;
    }


    public boolean setSurface(Surface newSurface)
    {
        if (Utils.isDebug) Utils.Log ( 5, className, "setSurface" );

        surface = newSurface;

        // try to initialize. Most probably the stream Init (with width/height) from portal generator has not arrived yet
        init ();
        return true;
    }


    public void start ()
    {
        synchronized (this) {
            if (started || CoDec == null)
                return;

            try {
                CoDec.start();

                started = true;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public void stop ()
    {
        synchronized (this) {
            if (!started || CoDec == null)
                return;

            try {
                CoDec.stop();

                started = true;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public void release()
    {
        if (Utils.isDebug) Utils.Log ( 5, className, "release" );

        initialized = false;

        stop ();

        synchronized (this)
        {
            if (CoDec != null) {
                CoDec.release();
                //CoDec.flush();
                CoDec = null;
            }
        }
        surface = null;
    }


    long buildPresentationTime ( int index ) {
        return 132 + index * 1000000 / 33;
    }

    abstract void outputCallback ( int outputStatus );


    boolean perform ( byte[] buffer, int offset, int dataLength )
    {
        if (Utils.isDebug) Utils.Log ( 8, className, "perform: Data size=" + dataLength );

        MediaCodec codec = CoDec;
        if (codec == null) {
            Utils.LogW ( className, "perform: No codec available!");
            return false;
        }

        int index = codec.dequeueInputBuffer ( timeoutUs );
        if( index < 0 ) {
            Utils.LogW ( className, "perform: Failed to dequeue an input buffer!");
            return false;
        }

        ByteBuffer dstBuf = null;

        if ( API21 )
            dstBuf = codec.getInputBuffer(index);
        else
            dstBuf = inputBuffers [ index ];

        if (dstBuf == null) {
            Utils.LogE ( className, "perform: Failed to query the input buffer!");
            return false;
        }

        //dstBuf.clear();
        dstBuf.position ( 0 );

        if (isEncoder) {
            //dstBuf.put(buffer, 0, dstBuf.capacity());
            dstBuf.put ( buffer, offset, dataLength <= dstBuf.capacity () ? dataLength : dstBuf.capacity () );
        }
        else {
            if ( dstBuf.capacity() < dataLength ) {
                Utils.LogW ( className, "perform: Destination buffer(" + dstBuf.capacity() + ") too small for received data unit(" + dataLength + ")!");
                return false;
            }
            dstBuf.put ( buffer, offset, dataLength );
        }
        //dstBuf.position(0);

        long presentationTimeUs = 0;
        if (isEncoder) {
            presentationTimeUs = buildPresentationTime(frameIndex);
            frameIndex++;
        }

        codec.queueInputBuffer(index, 0, dataLength, presentationTimeUs, 0);

        if (performOutput) {
            int outputStatus = 0;

        /*while (outputStatus >= 0) // while look extremely slows down decoding
        {
        }
        */
            outputStatus = codec.dequeueOutputBuffer ( bufferInfo, timeoutUs );
            if ( outputStatus >= 0 )
            {
                outputCallback ( outputStatus );
            }
            else if ( outputStatus == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED )
            {
                Utils.Log1 ( className, "perform: Output format changed!" );

                format = codec.getOutputFormat ();
            }
            else if( outputStatus == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED )
            {
                Utils.Log1 ( className, "perform: Output buffers changed!" );

                if ( !API21 )
                    outputBuffers = codec.getOutputBuffers ( );
            }
            //else break;
        }

        if (Utils.isDebug) Utils.Log ( 8, className, "perform: ok" );
        return true;
    }


}
