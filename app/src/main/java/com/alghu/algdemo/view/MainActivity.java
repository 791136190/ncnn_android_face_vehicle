package com.alghu.algdemo.view;

/**
 * @ClassName:     MainActivity
 * @Description:   MainActiviry to start all other things
 *
 * @author         alghu
 * @version        V1.0
 * @Date           2019.08.16
 */

import android.content.DialogInterface;
import android.content.pm.ActivityInfo;
import android.os.StrictMode;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;
import android.widget.Button;
import android.view.View;
import android.widget.Toast;

//import com.alghu.algdemo.CameraNcnnFragment;
import com.alghu.algdemo.R;
import com.alghu.algdemo.utils.Untils;
import com.alghu.algdemo.utils.Constant;

//public class MainActivity extends AppCompatActivity
public class MainActivity extends AppCompatActivity implements View.OnClickListener{

    private static final String TAG = MainActivity.class.getSimpleName();

    protected Button btn_npu = null;
    protected Button btn_gpu = null;
    protected Button btn_cpu = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        initView();

        Log.i(TAG,"onCreate");
    }

    private void initView() {
        // 保持横屏，保持常亮
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        // 启动主窗口
        setContentView(R.layout.activity_main);

        // 主窗口按键注册
        btn_npu = (Button) findViewById(R.id.bt_npu);
        btn_gpu = (Button) findViewById(R.id.bt_gpu);
        btn_cpu = (Button) findViewById(R.id.bt_cpu);
        btn_npu.setOnClickListener(this);
        btn_gpu.setOnClickListener(this);
        btn_cpu.setOnClickListener(this);

        // 加载jni库文件
        boolean isSoLoadSuccess = Untils.loadJNISo();
        if (isSoLoadSuccess) {
            Toast.makeText(this, "load jni so success.", Toast.LENGTH_SHORT).show();
        }
        else {
            Toast.makeText(this, "load jni so fail.", Toast.LENGTH_SHORT).show();
        }
    }

    private void run_device(int device_code){
        AlgForCamera new_camera = new AlgForCamera();
        Bundle bundle = new Bundle();
        bundle.putInt("run_on_where", device_code);//这里的values就是我们要传的值
        new_camera.setArguments(bundle);

        getSupportFragmentManager().beginTransaction()
                .replace(R.id.conter, new_camera)
                .addToBackStack(null)
                .commit();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.bt_npu:
                Toast.makeText(this, "demo will run on NPU", Toast.LENGTH_SHORT).show();

                run_device(Constant.RUN_ON_NPU);
                break;

            case R.id.bt_gpu:
                Toast.makeText(this, "demo will run on GPU", Toast.LENGTH_SHORT).show();

                run_device(Constant.RUN_ON_GPU);

                break;
            case R.id.bt_cpu:
                Toast.makeText(this, "demo will run on CPU", Toast.LENGTH_SHORT).show();

                run_device(Constant.RUN_ON_CPU);
                break;
        }

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

}
