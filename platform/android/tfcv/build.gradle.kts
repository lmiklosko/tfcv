plugins {
    alias(libs.plugins.android.library)
    alias(libs.plugins.jetbrains.kotlin.android)
    id("maven-publish")
}

android {
    namespace = "eu.digitalsoftminds.tfcv"

    defaultConfig {
        minSdk = 23
        compileSdk = 34
        aarMetadata {
            minCompileSdk = 23
        }

        ndk {
            abiFilters += listOf("x86", "x86_64", "armeabi-v7a", "arm64-v8a")
        }

        externalNativeBuild {
            cmake {
                // arguments("-Dtfcv_LIBRARY_TYPE=SHARED", "-DANDROID_STL=c++_shared", "-DCMAKE_VERBOSE_MAKEFILE=ON")
                targets("tfcv_static")
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("../../../CMakeLists.txt")
            version = "3.30.3"
        }
    }

    publishing {
        singleVariant("release") {
            withSourcesJar()
        }
    }

    tasks.matching { it.name.contains("test", ignoreCase = true)}
        .configureEach {
            enabled = false
        }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }

    kotlinOptions {
        jvmTarget = "1.8"
    }

    buildFeatures {
        prefabPublishing = true
    }

    prefab {
        create("tfcv_static") {
            headerOnly = false
            headers = "src/cxx/include/tfcv"

            libraryName = "libtfcv_static"
        }
    }
    packaging {
      jniLibs {
          excludes += listOf("**/libc++_shared.so")
      }
    }
}

dependencies {
}

publishing {
    publications {
        register<MavenPublication>("release") {
            groupId = "eu.lmiklosko"
            artifactId = "tfcv"
            version = "1.3.2-alpha03"

            afterEvaluate {
                from(components["release"])
            }
        }
    }

    repositories {
        maven {
            name = "lmiklosko"
            url = uri("https://maven.pkg.github.com/lmiklosko/tfcv/")
            credentials {
                username = project.findProperty("mavenUser") as String?
                password = project.findProperty("mavenPassword") as String?
            }
        }
    }
}
