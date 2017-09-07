package com.blueeagle.hellospine.views;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;

import com.blueeagle.hellospine.R;
import com.blueeagle.hellospine.gl.StickerSurfaceView;

import butterknife.BindView;
import butterknife.ButterKnife;

public class MainActivity extends AppCompatActivity {

    @BindView(R.id.sticker_surface_view)
    StickerSurfaceView mStickerSurfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ButterKnife.bind(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mStickerSurfaceView.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        mStickerSurfaceView.onPause();
    }
}
