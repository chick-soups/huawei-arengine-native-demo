/**
 * Copyright 2022. Huawei Technologies Co., Ltd. All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

package com.huawei.arengine.demos.cworld;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import com.huawei.hiar.exceptions.ARFatalException;

/**
 * This activity is used to redirect the user to AppGallery and install the AR Engine server.
 * This activity is called when the AR Engine is not installed.
 *
 * @author HW
 * @since 2020-10-17
 */
public class ConnectAppMarketActivity extends Activity {
    private static final String TAG = ConnectAppMarketActivity.class.getSimpleName();

    private static final String ACTION_HUAWEI_DOWNLOAD_QUIK = "com.huawei.appmarket.intent.action.AppDetail";

    private static final String HUAWEI_MARTKET_NAME = "com.huawei.appmarket";

    private static final String PACKAGE_NAME_KEY = "APP_PACKAGENAME";

    private static final String PACKAGENAME_ARSERVICE = "com.huawei.arengine.service";

    private AlertDialog.Builder dialog;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_connection_app_market);
        showSuggestiveDialog();
    }

    @Override
    protected void onResume() {
        if (dialog != null) {
            Log.d(TAG, "show dialog.");
            dialog.show();
        }
        super.onResume();
    }

    private void showSuggestiveDialog() {
        Log.d(TAG, "Show education dialog.");
        dialog = new AlertDialog.Builder(this);
        showAppMarket();
    }

    private void showAppMarket() {
        dialog.setMessage(R.string.arengine_install_app);
        dialog.setNegativeButton(R.string.arengine_cancel, (dialogInterface, which) -> {
            Log.d(TAG, "Show education showAppMarket.");
            finish();
        });
        dialog.setPositiveButton(R.string.arengine_install, (dialogInterface, which) -> {
            try {
                Log.d(TAG, "arengine_install onClick.");
                downLoadArServiceApp();
                finish();
            } catch (ActivityNotFoundException e) {
                throw new ARFatalException("Failed to launch ARInstallActivity");
            }
        });
        dialog.setOnCancelListener(dialogInterface -> finish());
    }

    private void downLoadArServiceApp() {
        try {
            Intent intent = new Intent(ACTION_HUAWEI_DOWNLOAD_QUIK);
            intent.putExtra(PACKAGE_NAME_KEY, PACKAGENAME_ARSERVICE);
            intent.setPackage(HUAWEI_MARTKET_NAME);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
        } catch (SecurityException e) {
            Log.w(TAG, "the target app has no permission of media");
        } catch (ActivityNotFoundException e) {
            Log.w(TAG, "the target activity is not found: " + e.getMessage());
        }
    }
}