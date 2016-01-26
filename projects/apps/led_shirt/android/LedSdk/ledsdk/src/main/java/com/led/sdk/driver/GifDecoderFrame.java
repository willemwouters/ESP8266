package com.led.sdk.driver;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.RectF;

import com.led.sdk.helpers.GifDecoder;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.nio.ByteBuffer;

/**
 * Created by wouters on 11/8/15.
 */
public class GifDecoderFrame {

    private int mGif;
    private boolean mKeepAspect = false;
    private int mId = 0;
    private int mRegister = 0;
    private int mBufferNum = 1;
    private Context mContext;
    public GifDecoderFrame(Context cnt, int id, int buf, int reg, int gif, boolean keepAspect) {
        mGif = gif;
        mId = id;
        mRegister = reg;
        mContext = cnt;
        mBufferNum = buf;
        mKeepAspect = keepAspect;
    }


    public Bitmap getResizedBitmap(Bitmap bm, int newWidth, int newHeight, boolean keepaspect) {
        int width = bm.getWidth();
        int height = bm.getHeight();




        // CREATE A MATRIX FOR THE MANIPULATION
        Matrix matrix = new Matrix();
        // RESIZE THE BIT MAP
        Bitmap resizedBitmap = null;

        if(!keepaspect) {
            float scaleWidth = ((float) newWidth) / width;
            float scaleHeight = ((float) newHeight) / height;
            matrix.postScale(scaleWidth, scaleHeight);
            resizedBitmap = Bitmap.createBitmap(bm, 0, 0, width, height, matrix, false);

        } else {
            matrix.setRectToRect(new RectF(0, 0, bm.getWidth(), bm.getHeight()), new RectF(0, 0, newWidth, newHeight), Matrix.ScaleToFit.CENTER);
            resizedBitmap = Bitmap.createBitmap(bm, 0, 0, width, height, matrix, false);


        }



        return resizedBitmap;
    }

    public ByteBuffer getBuffer() {
        ByteBuffer  mBuffer = ByteBuffer.allocate(1000);
        InputStream str = mContext.getResources().openRawResource(mGif);
        GifDecoder g = new GifDecoder();
        Bitmap b = null;
        try {
            int ret = g.read(str, str.available());
            ByteArrayOutputStream stream = new ByteArrayOutputStream();
            g.advance();
            b = g.getNextFrame();
        } catch (Exception e) {}


            mBuffer.clear();
            mBuffer.rewind();
            if (mId > 0) {
                mBuffer.put((byte) 0xFD);
            }
            mBuffer.put((byte) mRegister);
            mBuffer.put((byte) mBufferNum);

            if(b != null && !b.isRecycled()) {
                Bitmap r = getResizedBitmap(b, 15, 8, mKeepAspect);
                for (int i = 0; i < 15; i++) {
                    for (int y = 0; y < 8; y++) {
                        if(i >=  r.getWidth() || y >= r.getHeight()) {
                            mBuffer.put((byte) 0x00);
                            mBuffer.put((byte) 0x00);
                            mBuffer.put((byte) 0x00);
                        } else {
                            int colour = r.getPixel(i, 7- y);
                            int red = Color.red(colour);
                            int blue = Color.blue(colour);
                            int green = Color.green(colour);
                            mBuffer.put((byte) red);
                            mBuffer.put((byte) green);
                            mBuffer.put((byte) blue);
                        }
                        //Log.v("WW", "r:" + red + " b:" + blue + " g:" + green);
                    }
                }
            }


        return mBuffer;
    }
}
