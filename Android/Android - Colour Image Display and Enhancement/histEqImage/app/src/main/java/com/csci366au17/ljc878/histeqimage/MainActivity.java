package com.csci366au17.ljc878.histeqimage;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.Toast;


public class MainActivity extends Activity {
    private static final int REQUEST_CODE = 1;
    private Bitmap bitmap;
    private ImageView imageView;
    private Bitmap equalized;
    boolean isOriginal = true;
    boolean navDisplay = false;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        imageView = (ImageView) findViewById(R.id.result);
    }

    /* Image picking Intent*/
    public void pickImage(View View) {
        Intent intent = new Intent();
        intent.setType("image/*");
        intent.setAction(Intent.ACTION_GET_CONTENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        startActivityForResult(intent, REQUEST_CODE);
    }

    /*Return from image picking intent*/
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        InputStream stream = null;
        if (requestCode == REQUEST_CODE && resultCode == Activity.RESULT_OK)
            try {
                // recyle unused bitmaps
                if (bitmap != null) {
                    bitmap.recycle();
                }
                stream = getContentResolver().openInputStream(data.getData());
                bitmap = BitmapFactory.decodeStream(stream);
                /*BEGIN Histogram Processing*/
                histProcessing();
                /*END Histogram Processing*/
                imageView.setImageBitmap(bitmap);
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            } finally {
                if (stream != null)
                    try {
                        stream.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
            }
    }
    /**Assignment Algorithms */
    //Histogram algorithm
    public void histProcessing(){
        //TODO FIGURE OUT WHAT IS WRONG WITH THE HISTOGRAM
    //apply histogram equalisation to RGB channels separately
    /**DECLARATION of Variables*/
    int[] Pixel = new int[bitmap.getWidth() * bitmap.getHeight()];
    int height = bitmap.getHeight();
    int width = bitmap.getWidth();
    int red =0;
    int green =0;
    int blue =0;
    int alpha =0;
    int newcolor =0;
    int size = bitmap.getHeight() * bitmap.getWidth();
    /**END Variables*/
    //copy bitmap to equalized to ensure that they are identical before operation begins
    equalized=bitmap.copy(Bitmap.Config.ARGB_8888, true);
    //GET pixel Data from Bitmap
    bitmap.getPixels(Pixel, 0, bitmap.getWidth(), 0, 0,bitmap.getWidth(), bitmap.getHeight());
    // Get an image histogram - calculated values by R, G, B channels
    ArrayList<int[]> imageHist = imageHistogram(Pixel,width,height);
    //Get RGB channels histograms fro the normalized sum's
    ArrayList<int[]> histLUT = histogramEqualizationLUT(imageHist,width,height);

    //Set the values of the pixels to their new normalized values
    for( int i=0; i<size; i++){
        //iterate through pixel[]
        //TODO solve problem here (think it's an issue with usng red = histLUT.get(0) [red]
        alpha = Color.alpha(Pixel[i]);
        red = Color.red(Pixel[i]);
        green = Color.green(Pixel[i]);
        blue = Color.blue(Pixel[i]);
        red = histLUT.get(0)[red];
        green = histLUT.get(1)[green];
        blue = histLUT.get(2)[blue];
        //Encode the ARGB values back into a Color value
        newcolor = Color.argb(alpha,red,blue,green);
        //newcolor = (alpha & 0xff) << 24 | (red & 0xff) << 16 | (green & 0xff) << 16 | (blue & 0xff);
        //Store the new Color Value in the Array
        Pixel[i] = newcolor;
    }
    //set Bitmap equalized to have the new pixel values
    equalized.setPixels(Pixel,0,bitmap.getWidth(),0,0,bitmap.getWidth(),bitmap.getHeight());

        //Return the pixels back into a Bitmap
    }
    /*Create the R,G,B histograms for the image*/
    public static ArrayList<int[]> imageHistogram (int[] pixels, int width, int height){
        int[] rhistogram = new int[256];
        int[] ghistogram = new int[256];
        int[] bhistogram = new int[256];

        for(int i=0; i<rhistogram.length; i++) rhistogram[i] = 0;
        for(int i=0; i<ghistogram.length; i++) ghistogram[i] = 0;
        for(int i=0; i<bhistogram.length; i++) bhistogram[i] = 0;

        for(int i=0; i<width; i++) {
            for(int j=0; j<height; j++) {
                //Get to colour values
                int red = Color.red(pixels[i]);
                int green = Color.green(pixels[i]);
                int blue = Color.blue(pixels[i]);
                // Increase the historgram values for each color
                rhistogram[red]++; ghistogram[green]++; bhistogram[blue]++;
            }
        }
        //add histogram to ArrayList
        ArrayList<int[]> hist = new ArrayList<int[]>();
        hist.add(rhistogram);
        hist.add(ghistogram);
        hist.add(bhistogram);

        return hist;
    }
    /*Creates histogram Look Up Tables(LUT)*/
    private static ArrayList<int[]> histogramEqualizationLUT(ArrayList<int[]> imageHist, int width, int height) {

        // Create the lookup table
        ArrayList<int[]> imageLUT = new ArrayList<int[]>();

        // Fill the lookup table
        int[] rhistogram = new int[256];
        int[] ghistogram = new int[256];
        int[] bhistogram = new int[256];

        for(int i=0; i<rhistogram.length; i++) rhistogram[i] = 0;
        for(int i=0; i<ghistogram.length; i++) ghistogram[i] = 0;
        for(int i=0; i<bhistogram.length; i++) bhistogram[i] = 0;

        long sumr = 0;
        long sumg = 0;
        long sumb = 0;

        // Calculate the scale factor
        float scale_factor = (float) (255.0 / (width * height));
        /*Perform the equalisation for the R,G,B arrays
        For each Array get the next value
            multiply by the scale factor
                if it is greater than 255
                then it = 255; and assign it to the histogram;
        else assign it to the histogram
         */
        for(int i=0; i<rhistogram.length; i++) {
            sumr += imageHist.get(0)[i];
            int valr = (int) (sumr * scale_factor);
            if(valr > 255) {
                rhistogram[i] = 255;
            }
            else rhistogram[i] = valr;

            sumg += imageHist.get(1)[i];
            int valg = (int) (sumg * scale_factor);
            if(valg > 255) {
                ghistogram[i] = 255;
            }
            else ghistogram[i] = valg;

            sumb += imageHist.get(2)[i];
            int valb = (int) (sumb * scale_factor);
            if(valb > 255) {
                bhistogram[i] = 255;
            }
            else bhistogram[i] = valb;
        }
        //add to arraylist
        imageLUT.add(rhistogram);
        imageLUT.add(ghistogram);
        imageLUT.add(bhistogram);

        return imageLUT;

    }


    public void imageSelection(View view){
    /*
    Display original or processed image
    CHECK's a boolean variable to see which image is displayed
    */
    if (isOriginal == true){
        //THEN display histImage
        imageView.setImageBitmap(equalized);
        //Set original displayed False
        isOriginal = false;
    }
    else if (isOriginal == false){
        //THEN display original image
        imageView.setImageBitmap(bitmap);
        //SET original displayed True
        isOriginal = true;
    }
    }
    //Maybe force MAX Height on imageView
    public void dispMode(View view){
    //Display Mode - Scale to fit & No scaling(navigation Buttons required)
    if (navDisplay == false){
        //Centered without scaling
        imageView.setScaleType(ImageView.ScaleType.CENTER);
        Toast.makeText(getApplicationContext(), "Centered with no Scaling", Toast.LENGTH_SHORT).show();

        //Set the is scaled variable to to true
        navDisplay = true;
    }
    else if (navDisplay == true){
        //Centered with scaling
        imageView.setScaleType(ImageView.ScaleType.CENTER_INSIDE);
        Toast.makeText(getApplicationContext(), "Centered with Scaling", Toast.LENGTH_SHORT).show();
        //Set the is scaled variable to false
        navDisplay = false;
    }
    }
    /**IMAGE Navigation Controls**/
    public  void mleft(View view){
        //move image left (x horizontal, y vertical)
        float x = imageView.getX();
        x -= 5;
        imageView.setX(x);
    }
    public  void mright(View view){
        //move image right (x horizontal, y vertical)
        float x = imageView.getX();
        x += 5;
        imageView.setX(x);
    }
    public void mup(View view){
        //move image up horizontally
        float y = imageView.getY();
        y -= 5;
        imageView.setY(y);
    }
    public void mdown(View view){
        //move image up horizontally
        float y = imageView.getY();
        y += 5;
        imageView.setY(y);
    }
}