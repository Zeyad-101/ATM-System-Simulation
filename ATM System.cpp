#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cctype>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <conio.h> // Required for _getch() (Password masking)

using namespace std;

const string ClientFileName = "Client.txt";

struct stClient
{
    string Account_Number;
    string PIN_Code;
    string Name;
    string Phone;
    double Account_Balance = 0.0;
    bool Mark_Client_For_Delete = false;
};

stClient Current_Client;

//-------------------------------------------------
// Standard Screen Header (GUI Improvement)
//-------------------------------------------------
void Print_Header(string Title)
{
    system("cls"); // Clear screen
    cout << "\n============================================\n";
    cout << "\t     " << Title;
    cout << "\n============================================\n";
}

//-------------------------------------------------
// Helper: Mask Password Input (****)
//-------------------------------------------------
string Read_Password_Masked()
{
    string Password = "";
    char ch = 0;

    while ((ch = _getch()) != 13) // 13 is ASCII for Enter
    {
        if (ch == 8) // 8 is ASCII for Backspace
        {
            if (Password.length() > 0)
            {
                cout << "\b \b"; // Erase character visually
                Password.pop_back(); // Remove from string
            }
        }
        else
        {
            Password += ch;
            cout << '*'; // Print *
        }
    }
    cout << endl;
    return Password;
}

//-------------------------------------------------
// General Helpers
//-------------------------------------------------

string GetCurrentDateTime()
{
    time_t now = time(nullptr);
    tm localTime{};
    localtime_s(&localTime, &now);

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y  %H:%M:%S", &localTime);
    return string(buffer);
}

void Show_ATM_Main_Menu_Screen();
void Show_Welcome_Screen(); // Forward declaration for the start screen
void Login();
void Show_Sign_Up_Screen();

vector<string> Split_String(string S1, string Delimiter = "#//#")
{
    size_t Pos = 0;
    vector<string> vString;

    while ((Pos = S1.find(Delimiter)) != string::npos)
    {
        vString.push_back(S1.substr(0, Pos));
        S1.erase(0, Pos + Delimiter.length());
    }
    if (!S1.empty()) vString.push_back(S1);
    return vString;
}

stClient Convert_ClientDataLine_To_Record(string Line, string Delimiter = "#//#")
{
    vector<string> vRecord = Split_String(Line);
    stClient Client;
    Client.Account_Number = vRecord[0];
    Client.PIN_Code = vRecord[1];
    Client.Name = vRecord[2];
    Client.Phone = vRecord[3];
    Client.Account_Balance = stod(vRecord[4]);
    return Client;
}

string Convert_ClientData_Record_To_Line(stClient Client, string Delimiter = "#//#")
{
    return Client.Account_Number + Delimiter +
        Client.PIN_Code + Delimiter +
        Client.Name + Delimiter +
        Client.Phone + Delimiter +
        to_string(Client.Account_Balance);
}

vector<stClient> Load_ClientData_From_File(string FileName)
{
    fstream My_File;
    string Line;
    vector<stClient> vClient;

    My_File.open(FileName, ios::in);
    if (My_File.is_open())
    {
        while (getline(My_File, Line))
            vClient.push_back(Convert_ClientDataLine_To_Record(Line));
        My_File.close();
    }
    return vClient;
}

void Save_Client_Data_To_File(string FileName, vector<stClient>& vClient)
{
    fstream My_File(FileName, ios::out);
    for (stClient& Client : vClient)
        if (!Client.Mark_Client_For_Delete)
            My_File << Convert_ClientData_Record_To_Line(Client) << endl;
    My_File.close();
}

void Add_Data_Line_To_File(string FileName, string stDataLine)
{
    fstream My_File;
    My_File.open(FileName, ios::out | ios::app);
    if (My_File.is_open())
    {
        My_File << stDataLine << endl;
        My_File.close();
    }
}

bool Check_If_Account_Exists(string AccountNumber, string FileName)
{
    vector <stClient> vClients = Load_ClientData_From_File(FileName);
    for (stClient& C : vClients)
    {
        if (C.Account_Number == AccountNumber)
            return true;
    }
    return false;
}

bool Deposit_Balance_To_Client_By_Account_Number(string Account_Number, vector<stClient>& vClient, double Amount)
{
    for (stClient& Client : vClient)
    {
        if (Client.Account_Number == Account_Number)
        {
            Client.Account_Balance += Amount;
            Save_Client_Data_To_File(ClientFileName, vClient);
            return true;
        }
    }
    return false;
}

//-------------------------------------------------
// Sign Up Logic (New Feature)
//-------------------------------------------------

void Show_Sign_Up_Screen()
{
    Print_Header("Create New Account");

    stClient NewClient;
    string TempAccNum;

    // 1. Get Account Number and Validate Uniqueness
    do
    {
        cout << "Enter Account Number: ";
        getline(cin >> ws, TempAccNum);

        if (Check_If_Account_Exists(TempAccNum, ClientFileName))
        {
            cout << "\n[!] Account Number already exists. Please try another.\n";
        }
        else
        {
            NewClient.Account_Number = TempAccNum;
            break;
        }
    } while (true);

    // 2. Get Other Details
    cout << "Enter Your Name: ";
    getline(cin, NewClient.Name);

    cout << "Enter Phone Number: ";
    getline(cin, NewClient.Phone);

    cout << "Create a PIN Code: ";
    NewClient.PIN_Code = Read_Password_Masked();

    cout << "Enter Initial Deposit Balance: ";
    cin >> NewClient.Account_Balance;

    // 3. Save to File
    Add_Data_Line_To_File(ClientFileName, Convert_ClientData_Record_To_Line(NewClient));

    cout << "\n--------------------------------------------\n";
    cout << " Account Created Successfully!";
    cout << "\n--------------------------------------------\n";
    cout << "Press any key to go to Login...";
    system("pause>0");
    Login();
}

//-------------------------------------------------
// Withdraw / Deposit / Transfer
//-------------------------------------------------

short Read_Quick_Withdraw_Option()
{
    short Option;
    do
    {
        cout << "Choose Amount [1-9]: ";
        cin >> Option;
    } while (Option < 1 || Option > 9);
    return Option;
}

short Get_Quick_Withdraw_Amount(short Option)
{
    switch (Option)
    {
    case 1: return 20;
    case 2: return 50;
    case 3: return 100;
    case 4: return 200;
    case 5: return 400;
    case 6: return 600;
    case 7: return 800;
    case 8: return 1000;
    default: return 0;
    }
}

void Perform_Quick_Withdraw_Option(short Option)
{
    if (Option == 9) return;
    short Amount = Get_Quick_Withdraw_Amount(Option);

    if (Amount > Current_Client.Account_Balance)
    {
        cout << "\n[!] Amount Exceeds Balance!\n";
        system("pause>0");
        return;
    }

    vector<stClient> vClient = Load_ClientData_From_File(ClientFileName);
    Deposit_Balance_To_Client_By_Account_Number(Current_Client.Account_Number, vClient, -Amount);
    Current_Client.Account_Balance -= Amount;
    cout << "\n[+] Withdrawn Successfully. New Balance: " << Current_Client.Account_Balance << endl;
}

void Show_Quick_Withdraw_Screen()
{
    Print_Header("Quick Withdraw");
    cout << "\t[1] 20\t\t[2] 50\n";
    cout << "\t[3] 100\t\t[4] 200\n";
    cout << "\t[5] 400\t\t[6] 600\n";
    cout << "\t[7] 800\t\t[8] 1000\n";
    cout << "\t[9] Exit\n";
    cout << "============================================\n";
    cout << "Your Balance is: " << Current_Client.Account_Balance << endl;

    Perform_Quick_Withdraw_Option(Read_Quick_Withdraw_Option());
}

void Show_Normal_Withdraw_Screen()
{
    Print_Header("Normal Withdraw");
    int Amount;
    do
    {
        cout << "Enter Amount (Multiple of 5): ";
        cin >> Amount;
    } while (Amount % 5 != 0);

    if (Amount > Current_Client.Account_Balance)
    {
        cout << "\n[!] Amount Exceeds Balance!\n";
        system("pause>0");
        return;
    }

    vector<stClient> vClient = Load_ClientData_From_File(ClientFileName);
    Deposit_Balance_To_Client_By_Account_Number(Current_Client.Account_Number, vClient, -Amount);
    Current_Client.Account_Balance -= Amount;
    cout << "\n[+] Withdrawn Successfully. New Balance: " << Current_Client.Account_Balance << endl;
}

void Show_Deposit_Screen()
{
    Print_Header("Deposit Screen");
    double Amount;
    do
    {
        cout << "Enter Deposit Amount: ";
        cin >> Amount;
    } while (Amount <= 0);

    vector<stClient> vClient = Load_ClientData_From_File(ClientFileName);
    Deposit_Balance_To_Client_By_Account_Number(Current_Client.Account_Number, vClient, Amount);
    Current_Client.Account_Balance += Amount;
    cout << "\n[+] Deposit Successful. New Balance: " << Current_Client.Account_Balance << endl;
}

void Show_Transfer_Screen()
{
    Print_Header("Transfer Money");

    string DestAccountNumber;
    bool Found = false;
    vector<stClient> vClient = Load_ClientData_From_File(ClientFileName);
    stClient DestClient;

    cout << "Enter Account Number to Transfer To: ";
    cin >> DestAccountNumber;

    // Check Self Transfer
    if (DestAccountNumber == Current_Client.Account_Number)
    {
        cout << "\n[!] You cannot transfer to yourself.\n";
        return;
    }

    // Find Destination Client
    for (stClient& C : vClient)
    {
        if (C.Account_Number == DestAccountNumber)
        {
            DestClient = C;
            Found = true;
            break;
        }
    }

    if (!Found)
    {
        cout << "\n[!] Account Not Found.\n";
        return;
    }

    double Amount;
    cout << "Enter Transfer Amount: ";
    cin >> Amount;

    if (Amount > Current_Client.Account_Balance)
    {
        cout << "\n[!] Insufficient Balance.\n";
        return;
    }

    cout << "\nTransfer " << Amount << " to " << DestClient.Name << "? (y/n): ";
    char Confirm;
    cin >> Confirm;

    if (tolower(Confirm) == 'y')
    {
        // Update Vector
        for (stClient& C : vClient)
        {
            if (C.Account_Number == Current_Client.Account_Number)
                C.Account_Balance -= Amount;
            else if (C.Account_Number == DestAccountNumber)
                C.Account_Balance += Amount;
        }

        Save_Client_Data_To_File(ClientFileName, vClient);
        Current_Client.Account_Balance -= Amount;
        cout << "\n[+] Transfer Successful.\n";
        cout << "Your New Balance: " << Current_Client.Account_Balance << endl;
    }
}

void Show_Check_Balance_Screen()
{
    Print_Header("Check Balance");
    cout << "Client: " << Current_Client.Name << endl;
    cout << "Balance: " << Current_Client.Account_Balance << endl;
}

//-------------------------------------------------
// ATM Main Menu
//-------------------------------------------------

enum enATMMainMenuOptions
{
    eQuickWithdraw = 1, eNormalWithdraw = 2, eDeposit = 3,
    eTransfer = 4, eCheckBalance = 5, eLogout = 6
};

void Perform_ATM_Main_Menu_Option(enATMMainMenuOptions Option)
{
    switch (Option)
    {
    case eQuickWithdraw: Show_Quick_Withdraw_Screen(); break;
    case eNormalWithdraw: Show_Normal_Withdraw_Screen(); break;
    case eDeposit: Show_Deposit_Screen(); break;
    case eTransfer: Show_Transfer_Screen(); break;
    case eCheckBalance: Show_Check_Balance_Screen(); break;
    case eLogout: Show_Welcome_Screen(); return;
    }
    system("pause>0");
    Show_ATM_Main_Menu_Screen();
}

void Show_ATM_Main_Menu_Screen()
{
    system("cls");
    cout << "\n============================================\n";
    cout << "\t     ATM MAIN MENU\n";
    cout << "\n============================================\n";

    // INFO BLOCK
    cout << "  User: " << Current_Client.Name << endl;
    cout << "  Date: " << GetCurrentDateTime() << endl;

    cout << "============================================\n";
    cout << "[1] Quick Withdraw\n";
    cout << "[2] Normal Withdraw\n";
    cout << "[3] Deposit\n";
    cout << "[4] Transfer\n";
    cout << "[5] Check Balance\n";
    cout << "[6] Logout\n";
    cout << "============================================\n";

    int Option;
    cout << "Choose Option [1-6]: ";
    cin >> Option;
    Perform_ATM_Main_Menu_Option((enATMMainMenuOptions)Option);
}

//-------------------------------------------------
// Login & Welcome Screen
//-------------------------------------------------

bool Find_Client_By_Account_Number_And_PIN_Code(string Account_Number, string PIN_Code, stClient& Client)
{
    vector<stClient> vClient = Load_ClientData_From_File(ClientFileName);
    for (stClient& C : vClient)
        if (C.Account_Number == Account_Number && C.PIN_Code == PIN_Code)
        {
            Client = C;
            return true;
        }
    return false;
}

void Login()
{
    string Account_Number, PIN_Code;
    bool Login_Failed;

    do
    {
        Print_Header("Login Screen");
        cout << "Account Number: ";
        getline(cin >> ws, Account_Number);

        cout << "PIN Code: ";
        PIN_Code = Read_Password_Masked(); // Use masked input

        Login_Failed = !Find_Client_By_Account_Number_And_PIN_Code(Account_Number, PIN_Code, Current_Client);

        if (Login_Failed)
        {
            cout << "\n[!] Invalid Account or PIN! Try again.\n";
            system("pause");
        }

    } while (Login_Failed);

    Show_ATM_Main_Menu_Screen();
}

void Show_Welcome_Screen()
{
    while (true)
    {
        Print_Header("Welcome to ATM System");
        cout << "[1] Login\n";
        cout << "[2] Sign Up (New Account)\n";
        cout << "[3] Exit\n";
        cout << "============================================\n";
        cout << "Choose Option: ";

        int Option;
        cin >> Option;

        switch (Option)
        {
        case 1: Login(); break;
        case 2: Show_Sign_Up_Screen(); break;
        case 3: exit(0);
        default: cout << "Invalid Option.\n"; system("pause");
        }
    }
}

//-------------------------------------------------
// Main
//-------------------------------------------------

int main()
{
    system("color f3"); // Aqua on White background
    Show_Welcome_Screen();
    return 0;
}