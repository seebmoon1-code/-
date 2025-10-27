#include <iostream>
#include <string>
#include <cstdlib> // برای تابع system

using namespace std;

int main() {
    string file_path;

    // دریافت مسیر فایل از کاربر
    cout << "لطفاً مسیر کامل فایل .ts را وارد کنید: " << endl;
    getline(cin, file_path);

    // ساخت دستور برای پخش با VLC (می‌توانید به جای vlc از mplayer هم استفاده کنید)
    // برای مثال: vlc /path/to/your/file.ts
    string command = "vlc \"" + file_path + "\""; 

    cout << "در حال اجرای دستور: " << command << endl;

    // اجرای دستور در شل لینوکس
    int result = system(command.c_str());

    // بررسی نتیجه (اختیاری)
    if (result == 0) {
        cout << "پخش فایل با موفقیت انجام شد (یا پلیر با موفقیت اجرا شد)." << endl;
    } else {
        // اگر دستور موفق نبود یا VLC نصب نبود
        cerr << "خطا در اجرای پلیر. مطمئن شوید VLC نصب است." << endl;
        cerr << "کد خطا: " << result << endl;
    }

    return 0;
}
