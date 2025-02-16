#include <navigation.h>

#include <texture.h>

#include <vector>

#if defined(_WIN32)
#include <Windows.h>
#endif

static std::vector<std::string> history;
static unsigned int history_index;
static std::string current_directory;
std::vector<File> files;

static bool load_drives()
{
    //if (std::filesystem::path(current_directory).compare(std::filesystem::path(current_directory).root_path()) != 0) return false; // Check if current path is root of drive (e.g., "C:/")
    static std::string resources = "D:/Development/Repositories/Visual Studio/Files/Files/resources";
    static unsigned int ssd_texture = load_texture(resources + "/ssd_1.png").id;
    files.clear();
#if defined(_WIN32)
    DWORD drives = GetLogicalDrives();
    unsigned int bits = sizeof(DWORD) * CHAR_BIT;
    for (unsigned int i = 0; i < bits; i++)
    {
        if (!(drives & (1u << i))) continue;

        File file = {
            std::string({ (char)('A' + i), ':', '/' }),
            std::string({ (char)('A' + i), ':' }),
            ssd_texture
        };
        files.push_back(file);

        //std::filesystem::space_info space = std::filesystem::space(std::string({ (char)('A' + i), ':', '/' }));
        //printf("  %zu free of %zu\n", space.free, space.capacity);
    }
#endif
    return true;
}

static bool load_directory(const std::string& directory)
{
    if (directory.empty()) return load_drives();
    if (directory == current_directory) return false;
    if (!std::filesystem::is_directory(directory)) return false;
    static std::string resources = "D:/Development/Repositories/Visual Studio/Files/Files/resources";
    static unsigned int dir_texture = load_texture(resources + "/dir.png").id;
    static unsigned int file_texture = load_texture(resources + "/file.png").id;
    try
    {
        auto it = std::filesystem::directory_iterator(directory);
        files.clear();
        std::vector<std::filesystem::directory_entry> entries;
        for (const auto& entry : it)
        {
            entries.push_back(entry);
        }
        std::sort(entries.begin(), entries.end(), [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
            {
                bool a_dir = a.is_directory();
                bool b_dir = b.is_directory();
                if ((a_dir && b_dir) || (!a_dir && !b_dir))
                {
                    return _strcmpi(a.path().string().c_str(), b.path().string().c_str()) < 0;
                }
                return a_dir;
            });
        unsigned int index = 0;
        for (const auto& entry : entries)
        {
            unsigned int thumbnail = 0;
            if (entry.is_directory()) thumbnail = dir_texture;
            if (entry.is_regular_file()) thumbnail = file_texture;
            files.push_back({ entry.path().string(), entry.path().filename().string(), thumbnail });
        }
        return true;
    }
    catch (const std::filesystem::filesystem_error&) // Access denied
    {
        printf("Access denied: %s\n", directory.c_str());
        return false;
    }
}

bool navigate_directory(std::string directory)
{
    if (directory == "/") directory = "";
    auto c_path = std::filesystem::path(current_directory);
    auto path = std::filesystem::path(directory);
    //bool from_drive = c_path.compare(c_path.root_path()) == 0;
    //bool to_drive = path.compare(path.root_path()) == 0;
    if (path.is_relative())
    {
        path = std::filesystem::absolute(path);
        directory = path.string();
    }
    if (!load_directory(directory)) return false;
    current_directory = directory;
    if (!current_directory.empty()) std::filesystem::current_path(current_directory);
    else std::filesystem::current_path("C:/");

    if (history_index + 1 < history.size())
    {
        history.erase(history.begin() + history_index + 1, history.end());
    }
    history.push_back(directory);
    history_index = (unsigned int)history.size() - 1;
    return true;
}

bool navigate_back()
{
    if (history_index == 0) return false;
    const std::string& dir = history[history_index - 1];
    if (!load_directory(dir)) return false;
    history_index--;
    current_directory = dir;
    if (!current_directory.empty()) std::filesystem::current_path(current_directory);
    else std::filesystem::current_path("C:/");
    return true;
}

bool navigate_forward()
{
    if (history_index == history.size() - 1) return false;
    const std::string& dir = history[history_index + 1];
    if (!load_directory(dir)) return false;
    history_index++;
    current_directory = dir;
    if (!current_directory.empty()) std::filesystem::current_path(current_directory);
    else std::filesystem::current_path("C:/");
    return true;
}

bool navigate_up()
{
    bool up = navigate_directory(std::filesystem::path(current_directory).parent_path().string());
    return up ? up : navigate_directory("");
}
