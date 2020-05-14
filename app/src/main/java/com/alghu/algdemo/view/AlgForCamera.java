
package com.alghu.algdemo.view;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Point;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.provider.ContactsContract;
import android.provider.MediaStore;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.Fragment;
import android.support.v4.content.ContextCompat;
import android.support.v4.content.FileProvider;
import android.support.v4.os.EnvironmentCompat;
import android.util.Log;
//import android.widget.Toast;

import android.util.Size;
import android.view.LayoutInflater;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.Toast;

import com.alghu.algdemo.utils.AlgManager;
import com.alghu.algdemo.utils.Constant;
import com.alghu.algdemo.R;
import com.alghu.algdemo.utils.Untils;
import com.alghu.algdemo.utils.ModelManager;
import com.alghu.algdemo.utils.DetOutManager;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class AlgForCamera extends Fragment implements View.OnClickListener, ActivityCompat.OnRequestPermissionsResultCallback{

    protected static final String TAG = MainActivity.class.getSimpleName();

    protected ModelManager ModelInfo = null;

    protected Lock ModelLock = new ReentrantLock();// 锁对象

    protected int RunOnWhere = Constant.RUN_ON_CPU;

    protected int AlgMode = -1;

    protected int DataMode = -1;

    protected boolean DoFaceRecRecording = false;
    protected String FaceRecRecordingName = null;
    protected Map<String, float[]> FaceRecMap= new HashMap<String, float[]>();

    protected int CameraFacing = CameraCharacteristics.LENS_FACING_FRONT;

    protected AutoFitSurfaceView DrawSurface = null;

    protected Bitmap GlobalBitmapImg = null;
    protected byte[] GlobalYuv = null;

    protected Point DisplaySize = new Point();
    protected Size GlobalImageSize = new Size(Constant.MAX_PREVIEW_WIDTH, Constant.MAX_PREVIEW_HEIGHT);

    private HandlerThread mBackgroundThread = null;
    private Handler mBackgroundHandler = null;
    private ImageReader mImageReader = null;
    private Semaphore mCameraOpenCloseLock = new Semaphore(1);
    private CameraDevice mCameraDevice = null;
    private CaptureRequest.Builder mPreviewYUVRequestBuilder = null;
    private CameraCaptureSession mCaptureYUVSession = null;
    private CaptureRequest mPreviewYUVRequest = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Log.i(TAG,"onCreate");

        RunOnWhere = getArguments().getInt("run_on_where");
        Toast.makeText(getActivity(), "run_on_where(0:cpu,1:gpu,2:npu)->" + RunOnWhere, Toast.LENGTH_SHORT).show();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.alg_for_camera, container, false);
        view.findViewById(R.id.btInfo).setOnClickListener(this);
        view.findViewById(R.id.btChoice).setOnClickListener(this);
        view.findViewById(R.id.btCamera).setOnClickListener(this);
        view.findViewById(R.id.btFace).setOnClickListener(this);
        view.findViewById(R.id.btDlpr).setOnClickListener(this);
        view.findViewById(R.id.btHead).setOnClickListener(this);
        view.findViewById(R.id.btRec).setOnClickListener(this);

        DrawSurface = (AutoFitSurfaceView) view.findViewById(R.id.texture);
        DrawSurface.setAspectRatio(Constant.MAX_PREVIEW_WIDTH, Constant.MAX_PREVIEW_HEIGHT);

        return view;
    }

    @Override
    public void onClick(View view) {
        Log.i(TAG, "onClick " + view.getId());
        switch (view.getId()) {
            case R.id.btInfo: {
                Activity activity = getActivity();
                if (null != activity)
                {
                    Toast.makeText(activity, R.string.info_message, Toast.LENGTH_SHORT).show();
                }
                break;
            }

            case R.id.btChoice: {
                Activity activity = getActivity();
                if(null != activity)
                {
                    new AlertDialog.Builder(activity)
                            .setTitle(R.string.choice)
                            .setItems(new String[]{getString(R.string.image), getString(R.string.camera), getString(R.string.photo)},
                                    new DialogInterface.OnClickListener() {
                                        @Override
                                        public void onClick(DialogInterface dialog, int which) {
                                            switch (which)
                                            {
                                                case 0:
                                                {
                                                    checkImagePermission();
                                                }
                                                break;

                                                case 1:
                                                {
                                                    checkCameraPermission();
                                                }
                                                break;

                                                case 2:
                                                {
                                                    checkCapturePermission();
                                                }
                                                break;
                                            }
                                        }
                                    })
                            .show();

                }
                break;
            }

            case R.id.btCamera: {
                Activity activity = getActivity();
                if (null != activity)
                {
                    Toast.makeText(activity, R.string.change_camera, Toast.LENGTH_SHORT).show();
                    if (CameraFacing ==  CameraCharacteristics.LENS_FACING_BACK) {
                        CameraFacing = CameraCharacteristics.LENS_FACING_FRONT;
                    }
                    else {
                        CameraFacing = CameraCharacteristics.LENS_FACING_BACK;
                    }
                    if(DataMode == Constant.DATA_BY_CAMERA)
                    {
                        closeCamera();
                        openCamera();
                    }
                }
                break;
            }

            case R.id.btFace: {
                Activity activity = getActivity();
                if(null != activity)
                {
                    Toast.makeText(activity, R.string.change_face, Toast.LENGTH_SHORT).show();
                    DestoryAlg();
                    InitAlg(Constant.ALG_RUN_FACE);
                }

                break;
            }

            case R.id.btDlpr: {
                Activity activity = getActivity();
                if(null != activity)
                {
                    Toast.makeText(activity, R.string.change_lpr, Toast.LENGTH_SHORT).show();
                    DestoryAlg();
                    InitAlg(Constant.ALG_RUN_DLPR);
                }
                break;
            }

            case R.id.btHead:{
                Activity activity = getActivity();
                if(null != activity)
                {
                    Toast.makeText(activity, R.string.change_head, Toast.LENGTH_SHORT).show();
                    DestoryAlg();
                    InitAlg(Constant.ALG_RUN_HEAD);
                }
                break;
            }

            case R.id.btRec:{
                Activity activity = getActivity();
                if(null != activity)
                {
                    if (AlgMode == Constant.ALG_RUN_FACE && DataMode >= Constant.DATA_BY_IMAGE) {
                        Toast.makeText(activity, R.string.rec_info, Toast.LENGTH_SHORT).show();
                        DoFaceRecRecording = false;
                        FaceRecRecordingName = null;

                        final EditText et = new EditText(getActivity());
                        new AlertDialog.Builder(getActivity())
                                .setTitle("Input Name")
                                .setIcon(android.R.drawable.ic_dialog_info)
                                .setView(et)
                                .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {
                                        String input = et.getText().toString();
                                        if (input.equals("")) {
                                            Toast.makeText(getActivity(), "not support null input" + input, Toast.LENGTH_SHORT).show();
                                        } else {
                                            Toast.makeText(getActivity(), "your input is:" + input, Toast.LENGTH_SHORT).show();
                                            DoFaceRecRecording = true;
                                            FaceRecRecordingName = input;
                                        }
                                    }
                                })
                                .setNegativeButton("Cancel", null)
                                .show();
                    }
                    else {
                        Toast.makeText(activity, R.string.rec_warning, Toast.LENGTH_SHORT).show();
                    }
                }
            }
        }
    }

    private void checkImagePermission() {
        if (ContextCompat.checkSelfPermission(getActivity(), Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED &&
                ContextCompat.checkSelfPermission(getActivity(), Manifest.permission.CAMERA)
                        != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(getActivity(),
                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.CAMERA},
                    Constant.IMAGE_REQUEST_CODE);
        } else {
            Toast.makeText(getActivity(), "do image process", Toast.LENGTH_SHORT).show();
            chooseImageForAlg();
        }
    }

    private void checkCameraPermission() {
        if (ContextCompat.checkSelfPermission(getActivity(), Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED &&
                ContextCompat.checkSelfPermission(getActivity(), Manifest.permission.CAMERA)
                        != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(getActivity(),
                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.CAMERA},
                    Constant.CAMEAR_REQUEST_CODE);
        } else {
            Toast.makeText(getActivity(), "do camera process", Toast.LENGTH_SHORT).show();
            openCameraForAlg();
        }
    }

    private void checkCapturePermission() {
        if (ContextCompat.checkSelfPermission(getActivity(), Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED &&
                ContextCompat.checkSelfPermission(getActivity(), Manifest.permission.CAMERA)
                        != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(getActivity(),
                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.CAMERA},
                    Constant.CAPTURE_REQUEST_CODE);
        } else {
            Toast.makeText(getActivity(), "do capture process", Toast.LENGTH_SHORT).show();
            takeCaptureForAlg();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        if (requestCode == Constant.IMAGE_REQUEST_CODE) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED && grantResults[1] == PackageManager.PERMISSION_GRANTED) {
                Toast.makeText(getActivity(), "do image process", Toast.LENGTH_SHORT).show();
                chooseImageForAlg();
            } else {
                Toast.makeText(getActivity(), "Permission Denied", Toast.LENGTH_SHORT).show();
            }
        }

        if (requestCode == Constant.CAMEAR_REQUEST_CODE) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED && grantResults[1] == PackageManager.PERMISSION_GRANTED) {
                Toast.makeText(getActivity(), "do camera process", Toast.LENGTH_SHORT).show();
                openCameraForAlg();
            } else {
                Toast.makeText(getActivity(), "Permission Denied", Toast.LENGTH_SHORT).show();
            }
        }

        if (requestCode == Constant.CAPTURE_REQUEST_CODE) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED && grantResults[1] == PackageManager.PERMISSION_GRANTED) {
                Toast.makeText(getActivity(), "do capture process", Toast.LENGTH_SHORT).show();
                takeCaptureForAlg();
            } else {
                Toast.makeText(getActivity(), "Permission Denied", Toast.LENGTH_SHORT).show();
            }
        }
    }

    private void chooseImageForAlg() {
        Intent intent = new Intent(Intent.ACTION_PICK, null);
        intent.setDataAndType(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, "image/*");
        startActivityForResult(intent, Constant.IMAGE_REQUEST_CODE);
    }

    private void openCameraForAlg() {
//        Intent intent = new Intent(Intent.ACTION_PICK, null);
//        intent.setDataAndType(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, "image/*");
//        startActivityForResult(intent, Constant.CAMEAR_REQUEST_CODE);
        DataMode = Constant.DATA_BY_CAMERA;
    }

    //用于保存拍照图片的uri
    private Uri mCameraUri;

    // 用于保存图片的文件路径，Android 10以下使用图片路径访问图片
    private String mCameraImagePath;

    // 是否是Android 10以上手机
    private boolean isAndroidQ = Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.Q;

    /**
     * 创建图片地址uri,用于保存拍照后的照片 Android 10以后使用这种方法
     */
    private Uri createImageUri() {
        String status = Environment.getExternalStorageState();
        // 判断是否有SD卡,优先使用SD卡存储,当没有SD卡时使用手机存储
        if (status.equals(Environment.MEDIA_MOUNTED)) {
            return getActivity().getContentResolver().insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, new ContentValues());
        } else {
            return getActivity().getContentResolver().insert(MediaStore.Images.Media.INTERNAL_CONTENT_URI, new ContentValues());
        }
    }

    /**
     * 创建保存图片的文件
     */
    private File createImageFile() throws IOException {
        String imageName = new SimpleDateFormat("yyyyMMdd_HHmmss", Locale.getDefault()).format(new Date());
        File storageDir = getActivity().getExternalFilesDir(Environment.DIRECTORY_PICTURES);
        if (!storageDir.exists()) {
            storageDir.mkdir();
        }
        File tempFile = new File(storageDir, imageName);
        if (!Environment.MEDIA_MOUNTED.equals(EnvironmentCompat.getStorageState(tempFile))) {
            return null;
        }
        return tempFile;
    }

    private void takeCaptureForAlg() {
        Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        if(takePictureIntent.resolveActivity(getActivity().getPackageManager()) != null)
        {
            File photoFile = null;
            Uri photoUri = null;

            if (isAndroidQ) {
                // 适配android 10
                photoUri = createImageUri();
            } else {
                try {
                    photoFile = createImageFile();
                } catch (IOException e) {
                    e.printStackTrace();
                }

                if (photoFile != null) {
                    mCameraImagePath = photoFile.getAbsolutePath();
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                        //适配Android 7.0文件权限，通过FileProvider创建一个content类型的Uri
                        photoUri = FileProvider.getUriForFile(getActivity(), getActivity().getPackageName() + ".provider", photoFile);
                    } else {
                        photoUri = Uri.fromFile(photoFile);
                    }
                }
            }

            mCameraUri = photoUri;
            if (photoUri != null) {
                takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT, photoUri);
                takePictureIntent.addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                startActivityForResult(takePictureIntent, Constant.CAPTURE_REQUEST_CODE);
            }
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode == android.app.Activity.RESULT_OK && (data != null || requestCode == Constant.CAPTURE_REQUEST_CODE)) switch (requestCode) {
            case Constant.IMAGE_REQUEST_CODE:
                try {
                    Bitmap bitmap;
                    ContentResolver resolver = getActivity().getContentResolver();
                    Uri originalUri = data.getData();
                    bitmap = MediaStore.Images.Media.getBitmap(resolver, originalUri);
                    String[] proj = {MediaStore.Images.Media.DATA};
                    Cursor cursor = getActivity().managedQuery(originalUri, proj, null, null, null);
                    cursor.moveToFirst();
                    Bitmap rgba = bitmap.copy(Bitmap.Config.ARGB_8888, true);
                    GlobalBitmapImg = Bitmap.createScaledBitmap(rgba, GlobalImageSize.getWidth(), GlobalImageSize.getHeight(), false);
                    GlobalYuv = Untils.bitmap2Yuv420(GlobalBitmapImg);
                    DataMode = Constant.DATA_BY_IMAGE;
                } catch (IOException e) {
                    Log.e(TAG, e.toString());
                }

                break;
            case Constant.CAMEAR_REQUEST_CODE:
                Log.i("TAG","CAMEAR_REQUEST_CODE RETURN");
                DataMode = Constant.DATA_BY_CAMERA;

            case Constant.CAPTURE_REQUEST_CODE:
                Log.i("TAG","IMAGE_CAPTURE_REQUEST_CODE RETURN");

                if (isAndroidQ) {
                    // Android 10 使用图片uri加载
//                    ivPhoto.setImageURI(mCameraUri);
                    try {
                        ContentResolver cr = getActivity().getContentResolver();
                        InputStream input = cr.openInputStream(mCameraUri);
                        Bitmap Image = BitmapFactory.decodeStream(input);

                        GlobalBitmapImg = Bitmap.createScaledBitmap(Image, GlobalImageSize.getWidth(), GlobalImageSize.getHeight(), true);
                        GlobalYuv = Untils.bitmap2Yuv420(GlobalBitmapImg);
                        DataMode = Constant.DATA_BY_CAPTURE;

                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                } else {
                    // 使用图片路径加载
//                    ivPhoto.setImageBitmap(BitmapFactory.decodeFile(mCameraImagePath));
                    try {
                        ContentResolver cr = getActivity().getContentResolver();
                        Bitmap Image = BitmapFactory.decodeFile(mCameraImagePath);

                        GlobalBitmapImg = Bitmap.createScaledBitmap(Image, GlobalImageSize.getWidth(), GlobalImageSize.getHeight(), true);
                        GlobalYuv = Untils.bitmap2Yuv420(GlobalBitmapImg);
                        DataMode = Constant.DATA_BY_CAPTURE;

                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
                break;

            default:
                break;
        }
        else {
            Toast.makeText(getActivity(),
                    "Return without selecting pictures|Gallery has no pictures|Return without taking pictures", Toast.LENGTH_SHORT).show();
        }

    }

    private void ProcessImage(byte[] cur_camera_yuv)
    {
        try {
            if(AlgMode == Constant.ALG_RUN_FACE && DoFaceRecRecording)
            {
                DetOutManager det_out = RunAlg(cur_camera_yuv, GlobalImageSize.getWidth(), GlobalImageSize.getHeight());
                float[] Detect_result = det_out.getDetInfo();
                long FpsTime = det_out.getFpsTime();

                DrawSurface.DrawFaceInfo(cur_camera_yuv, GlobalImageSize.getWidth(), GlobalImageSize.getHeight(), Detect_result, FpsTime, Constant.MAX_FACE_NUM, Constant.MAX_FACE_LEN);

                Toast.makeText(getActivity(), "get name:"+FaceRecRecordingName, Toast.LENGTH_LONG).show();

                // 有目标时记录下来
                if(Detect_result[1] > 0) {
                    float[] rec = new float[Constant.MAX_FACE_LEN - 6 - 16];
                    System.arraycopy(Detect_result, 6 + 16, rec, 0, Constant.MAX_FACE_LEN - 6 - 16);
                    FaceRecMap.put(FaceRecRecordingName, rec);
                }
                DoFaceRecRecording = false;
                FaceRecRecordingName = null;
            }
            else if(AlgMode == Constant.ALG_RUN_FACE)
            {
                DetOutManager det_out = RunAlg(cur_camera_yuv, GlobalImageSize.getWidth(), GlobalImageSize.getHeight());
                float[] Detect_result = det_out.getDetInfo();
                long FpsTime = det_out.getFpsTime();

//                DrawSurface.DrawFaceInfo(cur_camera_yuv, GlobalImageSize.getWidth(), GlobalImageSize.getHeight(), Detect_result, FpsTime, Constant.MAX_FACE_NUM, Constant.MAX_FACE_LEN);
                DrawSurface.DrawFaceRec(cur_camera_yuv, GlobalImageSize.getWidth(), GlobalImageSize.getHeight(), Detect_result, FpsTime, Constant.MAX_FACE_NUM, Constant.MAX_FACE_LEN, FaceRecMap);
            }
            else if(AlgMode == Constant.ALG_RUN_DLPR)
            {
                DetOutManager det_out = RunAlg(cur_camera_yuv, GlobalImageSize.getWidth(), GlobalImageSize.getHeight());
                float[] Detect_result = det_out.getDetInfo();
                long FpsTime = det_out.getFpsTime();

                DrawSurface.DrawVehicleInfo(cur_camera_yuv, GlobalImageSize.getWidth(), GlobalImageSize.getHeight(), Detect_result, FpsTime, Constant.MAX_VEH_NUM, Constant.MAX_VEH_LEN);
            }
            else if(AlgMode == Constant.ALG_RUN_HEAD)
            {
                DetOutManager det_out = RunAlg(cur_camera_yuv, GlobalImageSize.getWidth(), GlobalImageSize.getHeight());
                float[] Detect_result = det_out.getDetInfo();
                long FpsTime = det_out.getFpsTime();

                DrawSurface.DrawHeadInfo(cur_camera_yuv, GlobalImageSize.getWidth(), GlobalImageSize.getHeight(), Detect_result, FpsTime, Constant.MAX_HEAD_NUM, Constant.MAX_HEAD_LEN);
            }
            else
            {
                DrawSurface.DrawYuv(cur_camera_yuv, GlobalImageSize.getWidth(), GlobalImageSize.getHeight());
            }
        } catch (Exception e) {
            e.printStackTrace();
            Toast.makeText(getContext(), e.toString(), Toast.LENGTH_SHORT).show();
        }
    }

    private final ImageReader.OnImageAvailableListener mOnImageAvailableListener
            = new ImageReader.OnImageAvailableListener() {

        @Override
        public void onImageAvailable(ImageReader reader) {
            Image im = reader.acquireNextImage();

            ByteBuffer bufferY = im.getPlanes()[0].getBuffer();
            ByteBuffer bufferU = im.getPlanes()[1].getBuffer();
            ByteBuffer bufferV = im.getPlanes()[2].getBuffer();

            ByteBuffer yuvbuffer = ByteBuffer.allocateDirect(bufferY.remaining() + bufferU.remaining() + bufferV.remaining());
            yuvbuffer.put(bufferY);
            yuvbuffer.put(bufferV);
            yuvbuffer.put(bufferU);

            im.close();

            byte[] cur_camera_yuv = yuvbuffer.array();

            // 抓拍和选图模式 使用全局yuv替换当前的yuv数据
            if(DataMode < Constant.DATA_BY_IMAGE)
            {
                return;
            }
            else if (DataMode != Constant.DATA_BY_CAMERA)
            {
                cur_camera_yuv = GlobalYuv;
            }

            ProcessImage(cur_camera_yuv);
        }

    };

    private String setUpCameraOutputs() {
        Activity activity = getActivity();
        CameraManager manager = (CameraManager) activity.getSystemService(Context.CAMERA_SERVICE);
        try {
            for (String cameraId : manager.getCameraIdList()) {
                CameraCharacteristics characteristics
                        = manager.getCameraCharacteristics(cameraId);

                // We don't use a front facing camera in this sample.
                Integer facing = characteristics.get(CameraCharacteristics.LENS_FACING);
//                if (facing != null && facing == CameraCharacteristics.LENS_FACING_BACK)
                if (facing != null && facing == CameraFacing)
                {
                    continue;
                }

                StreamConfigurationMap map = characteristics.get(
                        CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                if (map == null) {
                    continue;
                }

                List<Size> outlist = Arrays.asList(map.getOutputSizes(ImageFormat.YUV_420_888));
                Log.i(TAG, "The Camera Yuv Size Output list =" + outlist);

                // For still image captures, we use the largest available size.
                Size largest = Collections.max(outlist, new Untils.CompareSizesByArea());
                Log.i(TAG, "largest Output Size =" + largest);

                activity.getWindowManager().getDefaultDisplay().getSize(DisplaySize);
                int maxPreviewWidth = DisplaySize.x;
                int maxPreviewHeight = DisplaySize.y;
                int rotatedPreviewWidth = DisplaySize.x;
                int rotatedPreviewHeight = DisplaySize.y;

                if (maxPreviewWidth > Constant.MAX_PREVIEW_WIDTH) {
                    maxPreviewWidth = Constant.MAX_PREVIEW_WIDTH;
                }

                if (maxPreviewHeight > Constant.MAX_PREVIEW_HEIGHT) {
                    maxPreviewHeight = Constant.MAX_PREVIEW_HEIGHT;
                }

                // Danger, W.R.! Attempting to use too large a preview size could  exceed the camera
                // bus' bandwidth limitation, resulting in gorgeous previews but the storage of
                // garbage capture data.
                GlobalImageSize = Untils.chooseOptimalSize(map.getOutputSizes(SurfaceTexture.class),
                        rotatedPreviewWidth, rotatedPreviewHeight, maxPreviewWidth,
                        maxPreviewHeight, largest);

                //fix size 1280XN
                for(Size item : outlist)
                {
                    if(item.getWidth() == Constant.MAX_PREVIEW_WIDTH && item.getHeight() == Constant.MAX_PREVIEW_HEIGHT)
                    {
                        Log.i(TAG, "get  target best match is " +item.getWidth() + "X" + item.getHeight());
                        GlobalImageSize = item;
                        break;
                    }
                    else if(item.getWidth() == Constant.MAX_PREVIEW_WIDTH || item.getHeight() == Constant.MAX_PREVIEW_HEIGHT)
                    {
                        Log.i(TAG, "get  target resv is " +item.getWidth() + "X" + item.getHeight());
                        GlobalImageSize = item;
                    }
                }

//                DrawSurface.setAspectRatio(GlobalImageSize.getWidth(), GlobalImageSize.getHeight());

                mImageReader = ImageReader.newInstance(GlobalImageSize.getWidth(), GlobalImageSize.getHeight(),
                        ImageFormat.YUV_420_888, /*maxImages*/2);
                mImageReader.setOnImageAvailableListener(
                        mOnImageAvailableListener, mBackgroundHandler);

                return cameraId;
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        return null;
    }

    private CameraCaptureSession.CaptureCallback mCaptureCallback
            = new CameraCaptureSession.CaptureCallback() {

        private void process(CaptureResult result) {

        }

        @Override
        public void onCaptureProgressed(@NonNull CameraCaptureSession session,
                                        @NonNull CaptureRequest request,
                                        @NonNull CaptureResult partialResult) {
            process(partialResult);
        }

        @Override
        public void onCaptureCompleted(@NonNull CameraCaptureSession session,
                                       @NonNull CaptureRequest request,
                                       @NonNull TotalCaptureResult result) {
            process(result);
        }
    };

    private void createCameraYUVSession() {
        try {
            // We set up a CaptureRequest.Builder with the output Surface.
            mPreviewYUVRequestBuilder
                    = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            mPreviewYUVRequestBuilder.addTarget(mImageReader.getSurface());

            // Here, we create a CameraCaptureSession for camera preview.
            mCameraDevice.createCaptureSession(Arrays.asList(mImageReader.getSurface()),
                    new CameraCaptureSession.StateCallback() {

                        @Override
                        public void onConfigured(@NonNull CameraCaptureSession cameraCaptureSession) {
                            // The camera is already closed
                            if (null == mCameraDevice) {
                                return;
                            }

                            // When the session is ready, we start displaying the preview.
                            mCaptureYUVSession = cameraCaptureSession;
                            try {
                                // Finally, we start displaying the camera preview.
                                mPreviewYUVRequest = mPreviewYUVRequestBuilder.build();
                                mCaptureYUVSession.setRepeatingRequest(mPreviewYUVRequest,
                                        mCaptureCallback, mBackgroundHandler);
                            } catch (CameraAccessException e) {
                                e.printStackTrace();
                            }
                        }

                        @Override
                        public void onConfigureFailed(
                                @NonNull CameraCaptureSession cameraCaptureSession) {
                            Toast.makeText(getActivity(),"Failed", Toast.LENGTH_SHORT).show();
                        }
                    }, null
            );
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private final CameraDevice.StateCallback mStateCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice cameraDevice) {
            // This method is called when the camera is opened.  We start camera preview here.
            mCameraOpenCloseLock.release();
            mCameraDevice = cameraDevice;

            Log.i(TAG, "createCameraYUVSession");
            createCameraYUVSession();
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice cameraDevice) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            mCameraDevice = null;
        }

        @Override
        public void onError(@NonNull CameraDevice cameraDevice, int error) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            mCameraDevice = null;
            Activity activity = getActivity();
            if (null != activity) {
                activity.finish();
            }
        }

    };

    private void openCamera() {
        if (ContextCompat.checkSelfPermission(getActivity(), Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED &&
                ContextCompat.checkSelfPermission(getActivity(), Manifest.permission.CAMERA)
                        != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(getActivity(),
                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.CAMERA},
                    Constant.CAPTURE_REQUEST_CODE);
            return;
        }
        // mImageReader, mFlashSupported, mCameraId, mPreviewSize,
        // mSensorOrientation， mSurfaceView， mOnImageAvailableListener, mBackgroundHandler
        String CameraId = setUpCameraOutputs();

        // 这里会配置动态的旋转方向，因为固定了方向所以不配置也ok
//        configureTransform(width, height);

        Activity activity = getActivity();
        CameraManager manager = (CameraManager) activity.getSystemService(Context.CAMERA_SERVICE);
        try {
            if (!mCameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw new RuntimeException("Time out waiting to lock camera opening.");
            }
            manager.openCamera(CameraId, mStateCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera opening.", e);
        }
    }

    /**
     * Closes the current {@link CameraDevice}.
     */
    private void closeCamera() {
        try {
            mCameraOpenCloseLock.acquire();

            if (null != mCameraDevice) {
                mCameraDevice.close();
                mCameraDevice = null;
            }

            if (null != mImageReader) {
                mImageReader.close();
                mImageReader = null;
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera closing.", e);
        } finally {
            mCameraOpenCloseLock.release();
        }
    }

    private void startBackgroundThread() {
        mBackgroundThread = new HandlerThread("CameraBackground");
        mBackgroundThread.start();
        mBackgroundHandler = new Handler(mBackgroundThread.getLooper());
    }

    private void stopBackgroundThread() {
        mBackgroundThread.quitSafely();
        try {
            mBackgroundThread.join();
            mBackgroundThread = null;
            mBackgroundHandler = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onResume() {
        super.onResume();

        startBackgroundThread();
        openCamera();
    }

    @Override
    public void onPause() {
        super.onPause();

        stopBackgroundThread();
        closeCamera();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        DestoryAlg();
    }

    private void InitAlg(int algMode)
    {
        ModelLock.lock();

        ModelInfo = new ModelManager();
        // 创建临时路径存储模型
        File dir = getContext().getDir("models", Context.MODE_PRIVATE);
        String path = dir.getAbsolutePath() + File.separator ;
        ModelInfo.setModelSaveDir(path);

        ModelInfo.setRun_On_Where(RunOnWhere);

        if(algMode == Constant.ALG_RUN_FACE)
        {
            ModelInfo.setFaceDetModelParam("face_det.param");
            ModelInfo.setFaceDetModelData("face_det.bin");

            ModelInfo.setFaceAttrModelParam("face_attr.param");
            ModelInfo.setFaceAttrModelData("face_attr.bin");

            ModelInfo.setFaceRecModelParam("face_rec.param");
            ModelInfo.setFaceRecModelData("face_rec.bin");

            AssetManager am = getActivity().getAssets();

//            if(!Untils.isExistModelsInAppModels(ModelInfo.getFaceModelParam(), ModelInfo.getModelSaveDir())){
//                Untils.copyModelsFromAssetToAppModelsByBuffer(am, ModelInfo.getFaceModelParam(), ModelInfo.getModelSaveDir());
//            }
//            if(!Untils.isExistModelsInAppModels(ModelInfo.getFaceModelData(), ModelInfo.getModelSaveDir())){
//                Untils.copyModelsFromAssetToAppModelsByBuffer(am, ModelInfo.getFaceModelData(), ModelInfo.getModelSaveDir());
//            }
            Untils.copyModelsFromAssetToAppModelsByBuffer(am, ModelInfo.getFaceDetModelParam(), ModelInfo.getModelSaveDir());
            Untils.copyModelsFromAssetToAppModelsByBuffer(am, ModelInfo.getFaceDetModelData(), ModelInfo.getModelSaveDir());

            Untils.copyModelsFromAssetToAppModelsByBuffer(am, ModelInfo.getFaceAttrModelParam(), ModelInfo.getModelSaveDir());
            Untils.copyModelsFromAssetToAppModelsByBuffer(am, ModelInfo.getFaceAttrModelData(), ModelInfo.getModelSaveDir());

            Untils.copyModelsFromAssetToAppModelsByBuffer(am, ModelInfo.getFaceRecModelParam(), ModelInfo.getModelSaveDir());
            Untils.copyModelsFromAssetToAppModelsByBuffer(am, ModelInfo.getFaceRecModelData(), ModelInfo.getModelSaveDir());

            ModelInfo.setInput_C(3);
            ModelInfo.setInput_H(360);
            ModelInfo.setInput_W(480);

            AlgManager.InitFaceModelFromFile(
                    ModelInfo.getModelSaveDir()+ModelInfo.getFaceDetModelParam(),
                    ModelInfo.getModelSaveDir()+ModelInfo.getFaceDetModelData(),
                    ModelInfo.getModelSaveDir()+ModelInfo.getFaceAttrModelParam(),
                    ModelInfo.getModelSaveDir()+ModelInfo.getFaceAttrModelData(),
                    ModelInfo.getModelSaveDir()+ModelInfo.getFaceRecModelParam(),
                    ModelInfo.getModelSaveDir()+ModelInfo.getFaceRecModelData(),
                    ModelInfo.getRun_On_Where(), ModelInfo.getInput_W(), ModelInfo.getInput_H());

            AlgMode = Constant.ALG_RUN_FACE;
        }
        else if(algMode == Constant.ALG_RUN_DLPR)
        {
            ModelInfo.setVehDetModelParam("veh_det.param");
            ModelInfo.setVehDetModelData("veh_det.bin");

            ModelInfo.setPlateDetModelParam("plate_det.param");
            ModelInfo.setPlateDetModelData("plate_det.bin");

            ModelInfo.setPlateRecModelParam("plate_rec.param");
            ModelInfo.setPlateRecModelData("plate_rec.bin");

            AssetManager am = getActivity().getAssets();

            Untils.copyModelsFromAssetToAppModelsByBuffer(am, ModelInfo.getVehDetModelParam(), ModelInfo.getModelSaveDir());
            Untils.copyModelsFromAssetToAppModelsByBuffer(am, ModelInfo.getVehDetModelData(), ModelInfo.getModelSaveDir());

            Untils.copyModelsFromAssetToAppModelsByBuffer(am, ModelInfo.getPlateDetModelParam(), ModelInfo.getModelSaveDir());
            Untils.copyModelsFromAssetToAppModelsByBuffer(am, ModelInfo.getPlateDetModelData(), ModelInfo.getModelSaveDir());

            Untils.copyModelsFromAssetToAppModelsByBuffer(am, ModelInfo.getPlateRecModelParam(), ModelInfo.getModelSaveDir());
            Untils.copyModelsFromAssetToAppModelsByBuffer(am, ModelInfo.getPlateRecModelData(), ModelInfo.getModelSaveDir());

            ModelInfo.setInput_C(3);
            ModelInfo.setInput_H(360);
            ModelInfo.setInput_W(480);

            AlgManager.InitVehModelFromFile(
                    ModelInfo.getModelSaveDir()+ModelInfo.getVehDetModelParam(),
                    ModelInfo.getModelSaveDir()+ModelInfo.getVehDetModelData(),
                    ModelInfo.getModelSaveDir()+ModelInfo.getPlateDetModelParam(),
                    ModelInfo.getModelSaveDir()+ModelInfo.getPlateDetModelData(),
                    ModelInfo.getModelSaveDir()+ModelInfo.getPlateRecModelParam(),
                    ModelInfo.getModelSaveDir()+ModelInfo.getPlateRecModelData(),
                    ModelInfo.getRun_On_Where(), ModelInfo.getInput_W(), ModelInfo.getInput_H());

            AlgMode = Constant.ALG_RUN_DLPR;
        }
        else if(algMode == Constant.ALG_RUN_HEAD)
        {
            ModelInfo.setHeadDetModelParam("head_det.param");
            ModelInfo.setHeadDetModelData("head_det.bin");
            
            AssetManager am = getActivity().getAssets();
            
            Untils.copyModelsFromAssetToAppModelsByBuffer(am, ModelInfo.getHeadDetModelParam(), ModelInfo.getModelSaveDir());
            Untils.copyModelsFromAssetToAppModelsByBuffer(am, ModelInfo.getHeadDetModelData(), ModelInfo.getModelSaveDir());

            ModelInfo.setInput_C(3);
            ModelInfo.setInput_H(360);
            ModelInfo.setInput_W(480);

            AlgManager.InitHeadModelFromFile(
                    ModelInfo.getModelSaveDir()+ModelInfo.getHeadDetModelParam(),
                    ModelInfo.getModelSaveDir()+ModelInfo.getHeadDetModelData(),
                    ModelInfo.getRun_On_Where(), ModelInfo.getInput_W(), ModelInfo.getInput_H());

            AlgMode = Constant.ALG_RUN_HEAD;
        }
        
        ModelLock.unlock();
    }

    private void DestoryAlg()
    {
        if (ModelInfo == null || AlgMode < Constant.ALG_RUN_FACE)
        {
            return;
        }
        ModelLock.lock();

        if(AlgMode == Constant.ALG_RUN_FACE)
        {
            AlgManager.UinitFaceModel(ModelInfo.getRun_On_Where());
        }
        else if(AlgMode == Constant.ALG_RUN_DLPR)
        {
            AlgManager.UinitVehModel(ModelInfo.getRun_On_Where());
        }
        else if(AlgMode == Constant.ALG_RUN_HEAD)
        {
            AlgManager.UinitHeadModel(ModelInfo.getRun_On_Where());
        }

        AlgMode = -1;

        ModelLock.unlock();
    }

    private DetOutManager RunAlg(byte[] YuvData, int SrcWidth, int SrcHeight)
    {
        if (YuvData == null || AlgMode < Constant.ALG_RUN_FACE)
        {
            return null;
        }

        ModelLock.lock();

        long StartTime = System.currentTimeMillis();
        DetOutManager det_info = new DetOutManager();

        if(AlgMode == Constant.ALG_RUN_FACE) {
            float[] output = new float[Constant.MAX_FACE_NUM * Constant.MAX_FACE_LEN];
            float[] det_out = AlgManager.RunFaceModeByYuv(YuvData, SrcWidth, SrcHeight, output);
            det_info.setDetInfo(det_out);
        }
        else if (AlgMode == Constant.ALG_RUN_DLPR){
            float[] output = new float[Constant.MAX_VEH_NUM * Constant.MAX_VEH_LEN];
            float[] det_out = AlgManager.RunVehModeByYuv(YuvData, SrcWidth, SrcHeight, output);
            det_info.setDetInfo(det_out);
        }
        else if(AlgMode == Constant.ALG_RUN_HEAD){
            float[] output = new float[Constant.MAX_HEAD_NUM * Constant.MAX_HEAD_LEN];
            float[] det_out = AlgManager.RunHeadModeByYuv(YuvData, SrcWidth, SrcHeight, output);
            det_info.setDetInfo(det_out);
        }

        long FpsTime = System.currentTimeMillis() - StartTime;
        det_info.setFpsTime(FpsTime);

        ModelLock.unlock();
        return det_info;
    }
}
