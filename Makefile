ARCH?=arm64-v8a
KEYSTOREFILE:=key.keystore
ALIASNAME?=standkey
STOREPASS?=password
DNAME:="CN=orb.com, OU=ID, O=Orb, L=Paul, S=Batchelor, C=US"
BUILDDIR=build
APPNAME?=orb
APKFILE ?= $(APPNAME).apk
PACKAGENAME?=org.yourorg.$(APPNAME)
OS_NAME = linux-x86_64
ANDROIDSDK=$(ANDROID_HOME)
NDK?=$(firstword $(ANDROID_NDK) $(ANDROID_NDK_HOME) \
	$(wildcard $(ANDROIDSDK)/ndk/*) \
	$(wildcard $(ANDROIDSDK)/ndk-bundle/*))
ANDROIDVERSION?=23
ANDROIDTARGET?=$(ANDROIDVERSION)
BUILD_TOOLS?=$(lastword $(wildcard $(ANDROIDSDK)/build-tools/*))
AAPT:=$(BUILD_TOOLS)/aapt
ADB?=adb
LIBNAME=$(BUILDDIR)/lib/$(ARCH)/lib$(APPNAME).so


#DIR += sndkit soundpipe
#OBJ += sndkit/bigverb.o
#OBJ += sndkit/fmpair.o
#OBJ += sndkit/osc.o
#OBJ += sndkit/expmap.o
#OBJ += sndkit/swell.o
#OBJ += sndkit/dcblocker.o
#OBJ += soundpipe/smoother.o

CFLAGS+=-Inanovg -Ih -Isp -Iglue
OBJ+= nanovg/nanovg.o

DIR+= src
OBJ+=\
src/audio.o \
src/avatar.o \
src/color.o \
src/critter.o \
src/cstack.o \
src/fsm.o \
src/grid.o \
src/levels.o \
src/modal.o \
src/motion.o \
src/object.o \
src/orb_fsm.o \
src/revscm.o \
src/synth.o \
src/vals.o \
src/main.o \
src/run.o \

DIR+= sp sp/tangled
OBJ+=\
sp/base.o \
sp/delay.o \
sp/ftbl.o \
sp/randh.o \
sp/rspline.o \
sp/tenv.o \
sp/thresh.o \
sp/trand.o \
sp/butlp.o \
sp/vardelay.o \
sp/tangled/vardelay.o \
sp/metro.o \
sp/osc.o \
sp/tangled/osc.o \
sp/tenv.o \
sp/tenvx.o \
sp/dcblocker.o \
sp/tangled/dcblocker.o \
sp/mode.o \
sp/rline.o \
sp/tangled/rline.o \
sp/aux.o \
sp/fmpair.o \
sp/smoother.o \
sp/tangled/fmpair.o \

DIR += glue
OBJ += glue/android_native_app_glue.o
CFLAGS+=-DBUILD_ANDROID
CFLAGS+=-DNO_LIBSNDFILE

ifeq ($(ARCH), arm64-v8a)
CC=$(NDK)/toolchains/llvm/prebuilt/$(OS_NAME)/bin/aarch64-linux-android$(ANDROIDVERSION)-clang
CFLAGS+=-m64
LDFLAGS+=-L$(NDK)/toolchains/llvm/prebuilt/$(OS_NAME)/sysroot/usr/lib/aarch64-linux-android/$(ANDROIDVERSION)
endif

ifeq ($(ARCH), armeabi-v7a)
CC=$(NDK)/toolchains/llvm/prebuilt/$(OS_NAME)/bin/armv7a-linux-androideabi$(ANDROIDVERSION)-clang
CFLAGS+=-mfloat-abi=softfp -m32
LDFLAGS+=-L$(NDK)/toolchains/llvm/prebuilt/$(OS_NAME)/sysroot/usr/lib/arm-linux-androideabi/$(ANDROIDVERSION)
endif

ifeq ($(ARCH), x86)
CC=$(NDK)/toolchains/llvm/prebuilt/$(OS_NAME)/bin/i686-linux-android$(ANDROIDVERSION)-clang
CFLAGS+=-march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32
LDFLAGS+=-L$(NDK)/toolchains/llvm/prebuilt/$(OS_NAME)/sysroot/usr/lib/i686-linux-android/$(ANDROIDVERSION) 
endif

ifeq ($(ARCH), x86_64)
CC=$(NDK)/toolchains/llvm/prebuilt/$(OS_NAME)/bin/x86_64-linux-android$(ANDROIDVERSION)-clang
CFLAGS+=-march=x86-64 -msse4.2 -mpopcnt -m64 -mtune=intel
LDFLAGS+=-L$(NDK)/toolchains/llvm/prebuilt/$(OS_NAME)/sysroot/usr/lib/x86_64-linux-android/$(ANDROIDVERSION)
endif

CFLAGS+=-ffunction-sections
CFLAGS+= -fdata-sections -Wall -fvisibility=default
CFLAGS+=-DANDROID -DAPPNAME=\"$(APPNAME)\"
CFLAGS+= -O3
CFLAGS+=-I$(NDK)/sysroot/usr/include
CFLAGS+=-I$(NDK)/sysroot/usr/include/android
CFLAGS+=-I$(NDK)/toolchains/llvm/prebuilt/$(OS_NAME)/sysroot/usr/include/android
CFLAGS+=-fPIC
CFLAGS+=-DANDROIDVERSION=$(ANDROIDVERSION)
CFLAGS+=-I.
CFLAGS+=-g

LDFLAGS += -lm -lGLESv3 -lEGL -landroid -llog
LDFLAGS += -shared -uANativeActivity_onCreate
LDFLAGS += -lOpenSLES

ifdef LOFI
CFLAGS+=-DLOFI_MODE=1
endif
	

DIR+=nanovg

USER_DIR=$(addprefix $(BUILDDIR)/obj/$(ARCH)/, $(DIR))
CORE_DIR=$(addprefix $(BUILDDIR)/, \
	lib/$(ARCH) obj/$(ARCH) assets)

ALL_DIRS=$(USER_DIR) $(CORE_DIR)

#default: folders makecapk.apk
default: folders $(APKFILE)

$(ALL_DIRS):
	mkdir -p $@
folders: $(ALL_DIRS)

keystore: $(KEYSTOREFILE)

$(KEYSTOREFILE):
	keytool -genkey -v -keystore $(KEYSTOREFILE) \
	-alias $(ALIASNAME) -keyalg RSA -keysize 2048 \
	-validity 10000 -storepass $(STOREPASS) \
	-keypass $(STOREPASS) -dname $(DNAME)

$(BUILDDIR)/obj/$(ARCH)/%.o: %.c
	@echo "CC-$(ARCH) $<"
	@$(CC) -c $(CFLAGS) -o $@ $<


$(LIBNAME): $(addprefix $(BUILDDIR)/obj/$(ARCH)/, $(OBJ))
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(APKFILE): $(LIBNAME)
	$(RM) temp.apk
	$(RM) makecapk.apk
	$(AAPT) package -f -F temp.apk \
	-I $(ANDROIDSDK)/platforms/android-$(ANDROIDVERSION)/android.jar \
	-M AndroidManifest.xml -S res -A $(BUILDDIR)/assets \
	-v --target-sdk-version $(ANDROIDTARGET)
	unzip -o temp.apk -d $(BUILDDIR)
	cd $(BUILDDIR) && \
		zip -D9r ../makecapk.apk . && \
		zip -D0r ../makecapk.apk \
			./resources.arsc ./AndroidManifest.xml
	jarsigner -sigalg SHA1withRSA -digestalg SHA1 -verbose \
		-keystore $(KEYSTOREFILE) -storepass $(STOREPASS) \
		makecapk.apk $(ALIASNAME)
	$(RM) $(APKFILE)
	$(BUILD_TOOLS)/zipalign -v 4 makecapk.apk $(APKFILE)
	$(BUILD_TOOLS)/apksigner sign \
		--key-pass pass:$(STOREPASS) \
		--ks-pass pass:$(STOREPASS) \
		--ks $(KEYSTOREFILE) $(APKFILE)
	$(RM) temp.apk
	$(RM) makecapk.apk

clean:
	$(RM) $(APPNAME).apk
	$(RM) -r build

push : $(APKFILE)
	@echo "Installing" $(PACKAGENAME)
	$(ADB) install -r $(APKFILE)

run: push
	$(eval ACTIVITYNAME:=$(shell $(AAPT) dump badging $(APKFILE) | grep "launchable-activity" | cut -f 2 -d"'"))
	$(ADB) shell am start -n $(PACKAGENAME)/$(ACTIVITYNAME)

debug:
	$(eval ACTIVITYNAME:=$(shell $(AAPT) dump badging $(APKFILE) | grep "launchable-activity" | cut -f 2 -d"'"))
	$(ADB) -d shell am start -D -n $(PACKAGENAME)/$(ACTIVITYNAME)

monitor:
	$(ADB) logcat -c
	$(ADB) logcat -s "$(APPNAME)" "$(APPNAME):I"

lib: folders $(LIBNAME)
