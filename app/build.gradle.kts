plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.android)
}

android {
    namespace = "com.zu.videoplayer"
    compileSdk {
        version = release(36)
    }

    lint {
        abortOnError = false
        checkReleaseBuilds = false
    }

    defaultConfig {
        applicationId = "com.zu.videoplayer"
        minSdk = 24
        targetSdk = 36
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        externalNativeBuild {
            cmake {
                arguments("-DANDROID_PLATFORM=26", "-DANDROID_ARM_NEON=TRUE", "-DANDROID_STL=c++_shared", "-DENABLE_PERFORMANCE_MONITOR=TRUE")
                cppFlags("-std=c++17")
            }
        }

        ndk {
            abiFilters.addAll(arrayOf("armeabi-v7a", "arm64-v8a"))
        }
    }

    externalNativeBuild {
        cmake {
            path("CMakeLists.txt")
            version = "3.10.2"
        }
    }

    sourceSets {
        getByName("main") {
            jniLibs.srcDir("jniLibs")
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    kotlinOptions {
        jvmTarget = "11"
    }

    buildFeatures {
        prefab = true
        viewBinding = true
    }

    ndkVersion = "21.4.7075529"
}

dependencies {
    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.appcompat)
    implementation(libs.material)
    implementation(libs.androidx.activity)
    implementation(libs.androidx.constraintlayout)
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
    implementation("com.jakewharton.timber:timber:5.0.1")
    implementation("io.reactivex.rxjava2:rxandroid:2.1.1")
    implementation("io.reactivex.rxjava2:rxjava:2.2.16")
    implementation("com.google.oboe:oboe:1.6.1")
    implementation("com.guolindev.permissionx:permissionx:1.8.0")
    implementation("androidx.datastore:datastore-preferences:1.1.1")
}