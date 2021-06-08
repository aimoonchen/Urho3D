package io.urho3d.launcher;
//import org.libsdl.app.SDLActivity;
import android.Manifest;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.os.Bundle;

public class BGFXMainActivity extends android.app.NativeActivity {
    static {
        System.loadLibrary("Urho3DPlayer");
    }
//    @Override
//    protected void onCreate(Bundle savedInstanceState) {
//        // Call NativeActivity onCreate
//        super.onCreate(savedInstanceState);
//        SDLActivity.nativeSetupJNI();
//        SDLActivity.initialize();
//    }
}
