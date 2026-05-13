/*
 * =====================================================
 *   CYBERCAFE MANAGEMENT SYSTEM  v4
 *   OOP in C++ | Final Project
 * =====================================================
 */

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <limits>
using namespace std;

// ─── PEAK HOUR CHECK ──────────────────────────────────
bool isPeakHour() {
    time_t now = time(0);
    struct tm* t = localtime(&now);
    int h = t->tm_hour;
    return (h >= 8 && h < 10) || (h >= 14 && h < 16) || (h >= 20 && h < 22);
}
float applyPeak(float base) { return isPeakHour() ? base * 1.16f : base; }

// ─── SERVICE ITEM ─────────────────────────────────────
struct ServiceItem {
    string category, description;
    float  totalPrice;
    bool   peakApplied;
    // Hourly service support
    bool   isHourly;
    float  hourlyRate;   // rate/hr (peak-adjusted already)

    ServiceItem() : totalPrice(0), peakApplied(false), isHourly(false), hourlyRate(0) {}

    // Fixed or per-page service — price known at check-in
    ServiceItem(string cat, string desc, float price, bool peak)
        : category(cat), description(desc), totalPrice(price),
          peakApplied(peak), isHourly(false), hourlyRate(0) {}

    // Hourly service — price calculated at checkout from actual duration
    ServiceItem(string cat, string desc, float ratePerHr, bool peak, bool hourly)
        : category(cat), description(desc), totalPrice(0),
          peakApplied(peak), isHourly(hourly), hourlyRate(ratePerHr) {}

    // Called at checkout: multiply stored rate by actual session hours
    void finalizeHourly(float hours) {
        if (isHourly) totalPrice = hourlyRate * hours;
    }
};

// ─── WORKSTATION ──────────────────────────────────────
struct Workstation {
    int    id;
    string category, label, status;
    Workstation(int i, string cat, string lbl)
        : id(i), category(cat), label(lbl), status("Available") {}
    bool isAvailable() const { return status == "Available"; }
    void occupy()      { status = "In Use";      }
    void release()     { status = "Available";   }
    void maintenance() { status = "Maintenance"; }
    void display() const {
        cout << "    [" << id << "] " << label << "  |  " << status << "\n";
    }
};

// ─── 1. PERSON BASE CLASS ─────────────────────────────
class Person {
protected:
    int id; string name, password;
public:
    Person() : id(0), name(""), password("") {}
    Person(int i, string n, string p) : id(i), name(n), password(p) {}
    int    getId()   const { return id;   }
    string getName() const { return name; }
    bool   checkPassword(string p) const { return password == p; }
    virtual void   displayInfo() const {
        cout << "  ID   : " << id   << "\n";
        cout << "  Name : " << name << "\n";
    }
    virtual string getRole() const { return "Person"; }
    virtual ~Person() {}
};

// ─── 2. SESSION ───────────────────────────────────────
class Session {
private:
    int sessionId, customerId, wsId;
    string wsLabel;
    time_t startTime, endTime;
    bool   active;
    float  totalHours;
public:
    Session() : sessionId(0), customerId(0), wsId(0),
                startTime(0), endTime(0), active(false), totalHours(0) {}
    Session(int sId, int cId, int wid, string wlbl)
        : sessionId(sId), customerId(cId), wsId(wid), wsLabel(wlbl),
          active(true), totalHours(0) {
        startTime = time(0); endTime = 0;
    }
    int    getSessionId()  const { return sessionId;  }
    int    getWsId()       const { return wsId;       }
    string getWsLabel()    const { return wsLabel;    }
    bool   getIsActive()   const { return active;     }
    float  getTotalHours() const { return totalHours; }

    void endSession() {
        endTime    = time(0);
        active     = false;
        double sec = difftime(endTime, startTime);
        totalHours = (float)(sec / 3600.0);
        if (totalHours < 0.083f) totalHours = 0.083f;
    }
    int getElapsedMinutes() const {
        return (int)(difftime(time(0), startTime) / 60);
    }
    string getStartTimeStr() const {
        char buf[40]; struct tm* t = localtime(&startTime);
        strftime(buf, sizeof(buf), "%d-%m-%Y %H:%M:%S", t);
        return string(buf);
    }
    string getEndTimeStr() const {
        if (!endTime) return "N/A";
        char buf[40]; struct tm* t = localtime(&endTime);
        strftime(buf, sizeof(buf), "%d-%m-%Y %H:%M:%S", t);
        return string(buf);
    }
    void displaySummary() const {
        cout << "  Session ID   : " << sessionId         << "\n";
        cout << "  Workstation  : " << wsLabel           << "\n";
        cout << "  Start Time   : " << getStartTimeStr() << "\n";
        cout << "  End Time     : " << getEndTimeStr()   << "\n";
        cout << fixed << setprecision(2);
        cout << "  Duration     : " << totalHours << " hrs\n";
    }
};

// ─── 3. BILL ──────────────────────────────────────────
class Bill {
private:
    int billId, customerId;
    string customerName, cnic, contactNo, memberType, paymentMethod;
    vector<ServiceItem> services, foodOrders;
    float subtotal, discount, tax, grandTotal;
    bool  isPeak;
public:
    Bill() : billId(0), customerId(0), subtotal(0),
             discount(0), tax(0), grandTotal(0), isPeak(false) {}
    Bill(int bId, int cId, string cName, string cn, string ct,
         string mType, string pay,
         const vector<ServiceItem>& svcs,
         const vector<ServiceItem>& food, bool peak)
        : billId(bId), customerId(cId), customerName(cName),
          cnic(cn), contactNo(ct), memberType(mType),
          paymentMethod(pay), services(svcs), foodOrders(food), isPeak(peak) {
        subtotal = 0;
        for (auto& s : services)   subtotal += s.totalPrice;
        for (auto& f : foodOrders) subtotal += f.totalPrice;
        discount   = (mType == "Member") ? subtotal * 0.11f : 0.0f;
        float after = subtotal - discount;
        tax        = after * 0.05f;
        grandTotal = after + tax;
    }
    float  getGrandTotal() const { return grandTotal; }
    int    getBillId()     const { return billId;     }

    void printBill(const Session& sess) const {
        cout << "\n  =============================================\n";
        cout << "         CYBERCAFE MANAGEMENT SYSTEM\n";
        cout << "               INVOICE / BILL\n";
        cout << "  =============================================\n";
        cout << "  Bill ID      : " << billId        << "\n";
        cout << "  Customer ID  : " << customerId    << "\n";
        cout << "  Name         : " << customerName  << "\n";
        cout << "  CNIC         : " << cnic          << "\n";
        cout << "  Contact No   : " << contactNo     << "\n";
        cout << "  Member Type  : " << memberType    << "\n";
        cout << "  Payment Via  : " << paymentMethod << "\n";
        if (isPeak) cout << "  ** PEAK HOUR PRICING APPLIED (+16%) **\n";
        cout << "  ---------------------------------------------\n";
        cout << "  Workstation  : " << sess.getWsLabel()    << "\n";
        cout << "  Session Start: " << sess.getStartTimeStr() << "\n";
        cout << "  Session End  : " << sess.getEndTimeStr()   << "\n";
        cout << fixed << setprecision(2);
        cout << "  Duration     : " << sess.getTotalHours() << " hrs\n";
        cout << "  ---------------------------------------------\n";
        if (!services.empty()) {
            cout << "  ---- Services Availed ----\n";
            for (auto& s : services) {
                cout << "  " << s.category << " | " << s.description;
                if (s.isHourly) {
                    cout << "\n    Rate: Rs." << s.hourlyRate << "/hr"
                         << "  x  " << sess.getTotalHours() << " hrs";
                }
                cout << "  =>  Rs." << s.totalPrice << "\n";
            }
        }
        if (!foodOrders.empty()) {
            cout << "  ---- Food & Drinks ----\n";
            for (auto& f : foodOrders)
                cout << "  " << f.description << "  =>  Rs." << f.totalPrice << "\n";
        }
        cout << "  =============================================\n";
        cout << "  Subtotal     : Rs." << subtotal   << "\n";
        if (discount > 0)
        cout << "  Discount(11%): Rs." << discount   << "  [Member Benefit]\n";
        cout << "  Tax (5%)     : Rs." << tax        << "\n";
        cout << "  ---------------------------------------------\n";
        cout << "  GRAND TOTAL  : Rs." << grandTotal << "\n";
        cout << "  Payment Mode : " << paymentMethod << "\n";
        cout << "  =============================================\n";
        cout << "       Thank you for visiting CyberCafe!\n";
        cout << "  =============================================\n\n";
    }
};

// ─── 4. HISTORY RECORD ────────────────────────────────
struct HistoryRecord {
    int customerId, billId;
    string customerName, cnic, contactNo, memberType, paymentMethod;
    string wsLabel, startTime, endTime, checkoutDate;
    float totalHours, subtotal, discount, tax, grandTotal;
    bool  peakApplied;
    vector<ServiceItem> services, foodOrders;

    HistoryRecord() : customerId(0), billId(0), totalHours(0),
                      subtotal(0), discount(0), tax(0), grandTotal(0), peakApplied(false) {}

    HistoryRecord(int cId, string cName, string cn, string ct,
                  string mType, string pay,
                  const Session& sess, const Bill& bill,
                  const vector<ServiceItem>& svcs,
                  const vector<ServiceItem>& food, bool peak) {
        customerId   = cId;   customerName = cName;
        cnic         = cn;    contactNo    = ct;
        memberType   = mType; paymentMethod= pay;
        wsLabel      = sess.getWsLabel();
        startTime    = sess.getStartTimeStr();
        endTime      = sess.getEndTimeStr();
        totalHours   = sess.getTotalHours();
        billId       = bill.getBillId();
        grandTotal   = bill.getGrandTotal();
        services     = svcs;  foodOrders   = food;
        peakApplied  = peak;
        subtotal = 0;
        for (auto& s : svcs)  subtotal += s.totalPrice;
        for (auto& f : food)  subtotal += f.totalPrice;
        discount = (mType=="Member") ? subtotal*0.11f : 0.0f;
        float after = subtotal - discount;
        tax = after * 0.05f;
        time_t now = time(0); char buf[40];
        struct tm* t = localtime(&now);
        strftime(buf, sizeof(buf), "%d-%m-%Y %H:%M:%S", t);
        checkoutDate = string(buf);
    }

    void printInvoice() const {
        cout << "\n  =============================================\n";
        cout << "         CYBERCAFE MANAGEMENT SYSTEM\n";
        cout << "          INVOICE / BILL  [HISTORY]\n";
        cout << "  =============================================\n";
        cout << "  Bill ID      : " << billId        << "\n";
        cout << "  Customer ID  : " << customerId    << "\n";
        cout << "  Name         : " << customerName  << "\n";
        cout << "  CNIC         : " << cnic          << "\n";
        cout << "  Contact No   : " << contactNo     << "\n";
        cout << "  Member Type  : " << memberType    << "\n";
        cout << "  Payment Via  : " << paymentMethod << "\n";
        cout << "  Checkout On  : " << checkoutDate  << "\n";
        if (peakApplied) cout << "  ** PEAK HOUR PRICING APPLIED (+16%) **\n";
        cout << "  ---------------------------------------------\n";
        cout << "  Workstation  : " << wsLabel    << "\n";
        cout << "  Session Start: " << startTime  << "\n";
        cout << "  Session End  : " << endTime    << "\n";
        cout << fixed << setprecision(2);
        cout << "  Duration     : " << totalHours << " hrs\n";
        cout << "  ---------------------------------------------\n";
        if (!services.empty()) {
            cout << "  ---- Services ----\n";
            for (auto& s : services) {
                cout << "  " << s.category << " | " << s.description;
                if (s.isHourly) {
                    cout << "\n    Rate: Rs." << s.hourlyRate << "/hr"
                         << "  x  " << totalHours << " hrs";
                }
                cout << "  =>  Rs." << s.totalPrice << "\n";
            }
        }
        if (!foodOrders.empty()) {
            cout << "  ---- Food & Drinks ----\n";
            for (auto& f : foodOrders)
                cout << "  " << f.description << "  =>  Rs." << f.totalPrice << "\n";
        }
        cout << "  =============================================\n";
        cout << "  Subtotal     : Rs." << subtotal   << "\n";
        if (discount > 0)
        cout << "  Discount(11%): Rs." << discount   << "  [Member Benefit]\n";
        cout << "  Tax (5%)     : Rs." << tax        << "\n";
        cout << "  ---------------------------------------------\n";
        cout << "  GRAND TOTAL  : Rs." << grandTotal << "\n";
        cout << "  Payment Mode : " << paymentMethod << "\n";
        cout << "  =============================================\n";
        cout << "       Thank you for visiting CyberCafe!\n";
        cout << "  =============================================\n\n";
    }
};

// ─── 5. CUSTOMER ──────────────────────────────────────
class Customer : public Person {
private:
    string cnic, contactNo, memberType;
    bool   isLoggedIn;
    int    currentWsId;
    string currentWsLabel;
    Session* activeSession;
    vector<ServiceItem> sessionServices, sessionFood;
    bool   peakAtCheckIn;
    float  totalSpent;
    int    visitCount;
    Session             lastSession;
    vector<ServiceItem> lastServices, lastFood;
    bool                lastPeak;

public:
    Customer() : Person(), cnic(""), contactNo(""), memberType("New Guest"),
                 isLoggedIn(false), currentWsId(-1), activeSession(nullptr),
                 peakAtCheckIn(false), totalSpent(0), visitCount(0), lastPeak(false) {}
    Customer(int id, string nm, string cn, string ct, string mtype)
        : Person(id, nm, ""), cnic(cn), contactNo(ct), memberType(mtype),
          isLoggedIn(false), currentWsId(-1), activeSession(nullptr),
          peakAtCheckIn(false), totalSpent(0), visitCount(0), lastPeak(false) {}
    ~Customer() { if (activeSession) delete activeSession; }

    void displayInfo() const override {
        cout << "  --- Customer ---\n";
        Person::displayInfo();
        cout << "  CNIC        : " << cnic       << "\n";
        cout << "  Contact     : " << contactNo  << "\n";
        cout << "  Member Type : " << memberType << "\n";
        cout << fixed << setprecision(2);
        cout << "  Total Spent : Rs." << totalSpent << "\n";
        cout << "  Visits      : " << visitCount << "\n";
        cout << "  Status      : " << (isLoggedIn ? "Active" : "Idle") << "\n";
    }
    string getRole() const override { return "Customer"; }

    bool   getIsLoggedIn()      const { return isLoggedIn;      }
    int    getCurrentWsId()     const { return currentWsId;     }
    string getCurrentWsLabel()  const { return currentWsLabel;  }
    string getCnic()            const { return cnic;            }
    string getContactNo()       const { return contactNo;       }
    string getMemberType()      const { return memberType;      }
    float  getTotalSpent()      const { return totalSpent;      }
    int    getVisitCount()      const { return visitCount;      }
    const Session&             getLastSession()  const { return lastSession;  }
    const vector<ServiceItem>& getLastServices() const { return lastServices; }
    const vector<ServiceItem>& getLastFood()     const { return lastFood;     }
    bool                       getLastPeak()     const { return lastPeak;     }

    void startSession(int sId, int wsId, string wsLbl, bool peak) {
        if (activeSession) { cout << "  Already active!\n"; return; }
        currentWsId    = wsId;
        currentWsLabel = wsLbl;
        isLoggedIn     = true;
        peakAtCheckIn  = peak;
        visitCount++;
        activeSession = new Session(sId, id, wsId, wsLbl);
        cout << "\n  >> Session started: " << wsLbl << "\n";
        cout << "  >> Start: " << activeSession->getStartTimeStr() << "\n";
        if (peak) cout << "  ** PEAK HOUR: prices +16% applied **\n";
    }

    void addService(ServiceItem si) { sessionServices.push_back(si); }
    void addFood(ServiceItem fi)    { sessionFood.push_back(fi);     }

    int getElapsedMinutes() const {
        return activeSession ? activeSession->getElapsedMinutes() : 0;
    }

    Bill endSession(int billId, string payMethod) {
        if (!activeSession) { cout << "  No active session!\n"; return Bill(); }
        activeSession->endSession();

        float actualHours = activeSession->getTotalHours();

        // Finalize hourly services using actual session duration
        for (auto& s : sessionServices)
            if (s.isHourly) s.finalizeHourly(actualHours);

        lastSession  = *activeSession;
        lastServices = sessionServices;
        lastFood     = sessionFood;
        lastPeak     = peakAtCheckIn;
        Bill bill(billId, id, name, cnic, contactNo, memberType, payMethod,
                  sessionServices, sessionFood, peakAtCheckIn);
        bill.printBill(*activeSession);
        totalSpent += bill.getGrandTotal();
        isLoggedIn  = false;
        currentWsId = -1;
        currentWsLabel = "";
        delete activeSession; activeSession = nullptr;
        sessionServices.clear(); sessionFood.clear();
        peakAtCheckIn = false;
        return bill;
    }
};

// ─── 6. WORKSTATION MANAGER ───────────────────────────
class WorkstationManager {
private:
    vector<Workstation> stations;
public:
    WorkstationManager() {
        int id = 1;
        for (int i=1;i<=9;i++,id++) stations.push_back(Workstation(id,"Public/Usage PCs","Public PC #"+to_string(i)));
        for (int i=1;i<=6;i++,id++) stations.push_back(Workstation(id,"Gaming PCs","Gaming PC #"+to_string(i)));
        for (int i=1;i<=6;i++,id++) stations.push_back(Workstation(id,"Designing PCs","Designing PC #"+to_string(i)));
        for (int i=1;i<=4;i++,id++) stations.push_back(Workstation(id,"IT/Software Benches","IT Bench #"+to_string(i)));
        for (int i=1;i<=9;i++,id++) stations.push_back(Workstation(id,"Media/Printing Machines","Media Machine #"+to_string(i)));
        for (int i=1;i<=4;i++,id++) stations.push_back(Workstation(id,"Professional Rooms","Pro Room #"+to_string(i)));
    }

    void viewAll() {
        int choice;
        do {
            cout << "\n  ===== VIEW WORK STATIONS =====\n";
            cout << "  1. Public/Usage PCs        (9)\n";
            cout << "  2. Gaming PCs              (6)\n";
            cout << "  3. Designing PCs           (6)\n";
            cout << "  4. IT/Software Benches     (4)\n";
            cout << "  5. Media/Printing Machines (9)\n";
            cout << "  6. Professional Rooms      (4)\n";
            cout << "  0. Back\n";
            cout << "  Choice: "; cin >> choice;
            if (choice == 0) break;
            string cat;
            switch(choice){
                case 1: cat="Public/Usage PCs";        break;
                case 2: cat="Gaming PCs";              break;
                case 3: cat="Designing PCs";           break;
                case 4: cat="IT/Software Benches";     break;
                case 5: cat="Media/Printing Machines"; break;
                case 6: cat="Professional Rooms";      break;
                default: cout<<"  Invalid!\n"; continue;
            }
            cout << "\n  ----- " << cat << " -----\n";
            for (auto& ws : stations) if (ws.category==cat) ws.display();
            cout << "  ----------------------\n";
        } while(true);
    }

    void setMaintenance() {
        cout << "  Enter Workstation ID: "; int id; cin >> id;
        for (auto& ws : stations)
            if (ws.id==id) { ws.maintenance(); cout << "  >> Set to Maintenance.\n"; return; }
        cout << "  Not found!\n";
    }
    void setAvailable() {
        cout << "  Enter Workstation ID: "; int id; cin >> id;
        for (auto& ws : stations)
            if (ws.id==id) { ws.release(); cout << "  >> Set to Available.\n"; return; }
        cout << "  Not found!\n";
    }

    Workstation* findById(int id) {
        for (auto& ws : stations) if (ws.id==id) return &ws;
        return nullptr;
    }

    vector<Workstation*> availableInCategory(string cat) {
        vector<Workstation*> res;
        for (auto& ws : stations)
            if (ws.category==cat && ws.isAvailable()) res.push_back(&ws);
        return res;
    }

    int totalAvailable() const {
        int cnt=0; for (auto& ws:stations) if (ws.isAvailable()) cnt++; return cnt;
    }
};

// ─── SERVICE CATALOG HELPERS ──────────────────────────
struct SubService { string name; float basePrice; bool perPage, perHour; string unit; };
struct SvcCat     { string name; vector<SubService> items; };

vector<SvcCat> buildCatalog() {
    vector<SvcCat> c;
    c.push_back({"Computer Usage Services",    {{"Internet Browsing",49,false,true,"hour"},{"Online Form Filling",99,false,false,"fixed"},{"Job Application",149,false,false,"fixed"}}});
    c.push_back({"Printing Services",          {{"Black & White Print",9,true,false,"page"},{"Color Print",39,true,false,"page"},{"Photo Print (4x6)",99,true,false,"page"}}});
    c.push_back({"Scanning & Copying",         {{"Scanning",19,true,false,"page"},{"Photocopy B/W",5,true,false,"page"},{"Photocopy Color",25,true,false,"page"}}});
    c.push_back({"Documentation Services",     {{"CV / Resume",449,false,false,"fixed"},{"Cover Letter",249,false,false,"fixed"},{"Assignment Typing",49,true,false,"page"},{"Thesis Formatting",1599,false,false,"fixed"},{"Document Editing",199,false,false,"fixed"}}});
    c.push_back({"Online Services",            {{"Govt Form Submission",249,false,false,"fixed"},{"NADRA Assistance",299,false,false,"fixed"},{"Passport Application",449,false,false,"fixed"},{"Exam Form Submission",149,false,false,"fixed"}}});
    c.push_back({"Designing Services",         {{"Typography Logo",990,false,false,"fixed"},{"Symbolic Logo",1490,false,false,"fixed"},{"Combination Logo",1990,false,false,"fixed"},{"Visiting Card Design",390,false,false,"fixed"},{"Flex/Banner Design",660,false,false,"fixed"},{"Social Media Post",249,false,false,"fixed"}}});
    c.push_back({"Gaming PCs",                 {{"Standard Gaming PC",290,false,true,"hour"},{"High-End Gaming PC",599,false,true,"hour"},{"VIP Gaming (Headphones+Chair)",1190,false,true,"hour"}}});
    c.push_back({"Meeting Rooms",              {{"Small Room (2-4 persons)",490,false,true,"hour"},{"Medium Room (5-8 persons)",890,false,true,"hour"},{"Conference Room (10+)",1290,false,true,"hour"},{"Projector Setup (Extra)",250,false,true,"hour"}}});
    c.push_back({"Software & IT Services",     {{"Windows Installation",190,false,false,"fixed"},{"Software Installation",330,false,false,"fixed"},{"Virus Removal",690,false,false,"fixed"},{"Data Recovery",1990,false,false,"fixed"}}});
    c.push_back({"Photo & Media Services",     {{"Passport Size Photos",90,false,false,"fixed"},{"Photo Editing",190,false,false,"fixed"},{"Video Download",149,false,false,"fixed"}}});
    c.push_back({"Utility & Payment Services", {{"Bill Payment",15,false,false,"fixed"},{"Ticket Booking",90,false,false,"fixed"}}});
    return c;
}

vector<ServiceItem> pickServices(bool peak) {
    vector<SvcCat> catalog = buildCatalog();
    vector<ServiceItem> chosen;
    bool pickCat = true;
    while (pickCat) {
        cout << "\n  ===== SERVICE CATEGORIES =====\n";
        for (int i=0;i<(int)catalog.size();i++)
            cout << "  " << (i+1) << ". " << catalog[i].name << "\n";
        cout << "  0. Done\n  Choice: ";
        int cc; cin >> cc;
        if (cc==0) break;
        if (cc<1||cc>(int)catalog.size()) { cout << "  Invalid!\n"; continue; }
        SvcCat& sel = catalog[cc-1];
        bool pickItem = true;
        while (pickItem) {
            cout << "\n  --- " << sel.name;
            if (peak) cout << "  [PEAK +16%]";
            cout << " ---\n";
            for (int j=0;j<(int)sel.items.size();j++) {
                float p = peak ? sel.items[j].basePrice*1.16f : sel.items[j].basePrice;
                cout << "  " << (j+1) << ". " << sel.items[j].name
                     << "  =>  Rs." << fixed << setprecision(0) << p;
                if (sel.items[j].perPage) cout << "/page";
                if (sel.items[j].perHour) cout << "/hr  [billed by session time]";
                cout << "\n";
            }
            cout << "  0. Back\n  Choice: ";
            int ic; cin >> ic;
            if (ic==0) { pickItem=false; break; }
            if (ic<1||ic>(int)sel.items.size()) { cout << "  Invalid!\n"; continue; }
            SubService& sub = sel.items[ic-1];
            float unitP = peak ? sub.basePrice*1.16f : sub.basePrice;

            if (sub.perHour) {
                // ── Hourly: just record rate, price finalized at checkout ──
                cout << "  >> " << sub.name << " added at Rs."
                     << fixed << setprecision(2) << unitP
                     << "/hr  (will be calculated from actual session time)\n";
                chosen.push_back(ServiceItem(sel.name, sub.name, unitP, peak, true));

            } else if (sub.perPage) {
                // ── Per-page: ask quantity now ────────────────────────────
                cout << "  Number of pages: "; int qty; cin >> qty;
                float total = unitP * qty;
                cout << "  >> " << sub.name << " x" << qty
                     << " = Rs." << fixed << setprecision(2) << total << "\n";
                chosen.push_back(ServiceItem(sel.name, sub.name, total, peak));

            } else {
                // ── Fixed price ───────────────────────────────────────────
                cout << "  >> " << sub.name << " = Rs."
                     << fixed << setprecision(2) << unitP << "\n";
                chosen.push_back(ServiceItem(sel.name, sub.name, unitP, peak));
            }

            cout << "  Add more from this category? (1=Yes/0=No): "; int m; cin >> m;
            if (m!=1) pickItem=false;
        }
        cout << "  Add another category? (1=Yes/0=No): "; int mc; cin >> mc;
        if (mc!=1) pickCat=false;
    }
    return chosen;
}

vector<ServiceItem> pickFood(bool peak) {
    vector<ServiceItem> food;
    string names[] = {"Tea","Coffee","Soft Drink","Water","Samosa","Sandwich","Burger","Chips"};
    float  prices[]= {30,50,60,20,25,80,150,40};
    cout << "\n  ======= CAFE MENU =======";
    if (peak) cout << "  [PEAK +16%]";
    cout << "\n";
    for (int i=0;i<8;i++) {
        float p = peak ? prices[i]*1.16f : prices[i];
        cout << "  " << (i+1) << ". " << names[i] << "  Rs." << fixed << setprecision(0) << p << "\n";
    }
    cout << "  0. No thanks\n";
    int c;
    while (true) {
        cout << "  Select (0=done): "; cin >> c;
        if (c==0) break;
        if (c<1||c>8) { cout << "  Invalid!\n"; continue; }
        int qty; cout << "  Qty: "; cin >> qty;
        float p = peak ? prices[c-1]*1.16f : prices[c-1];
        float total = p * qty;
        cout << "  >> " << qty << "x " << names[c-1] << " = Rs." << fixed << setprecision(2) << total << "\n";
        string desc = to_string(qty) + "x " + names[c-1];
        food.push_back(ServiceItem("Food/Drink", desc, total, peak));
    }
    return food;
}

string pickPaymentMethod() {
    cout << "\n  ===== PAYMENT METHOD =====\n";
    cout << "  1. Cash\n  2. Credit Card\n  3. Debit Card\n  4. JazzCash\n  5. EasyPaisa\n";
    cout << "  Choice: "; int c; cin >> c;
    switch(c) {
        case 1: return "Cash";
        case 2: return "Credit Card";
        case 3: return "Debit Card";
        case 4: return "JazzCash";
        case 5: return "EasyPaisa";
        default: return "Cash";
    }
}

// ─── 7. ADMIN ─────────────────────────────────────────
class Admin : public Person {
private:
    vector<Customer>      customers;
    vector<HistoryRecord> history;
    WorkstationManager    wsManager;
    float totalRevenue;
    int   nextCustomerId, nextSessionId, nextBillId;

public:
    Admin(int id, string nm, string pw)
        : Person(id, nm, pw), totalRevenue(0),
          nextCustomerId(100), nextSessionId(1), nextBillId(1) {}

    void displayInfo() const override {
        Person::displayInfo();
        cout << fixed << setprecision(2);
        cout << "  Revenue: Rs." << totalRevenue << "\n";
    }
    string getRole() const override { return "Admin"; }

    void workstationMenu() {
        int c;
        do {
            cout << "\n  ===== WORK STATION MANAGEMENT =====\n";
            cout << "  1. View All Work Stations\n";
            cout << "  2. Set to Maintenance\n";
            cout << "  3. Set to Available\n";
            cout << "  0. Back\n";
            cout << "  Choice: "; cin >> c;
            switch(c) {
                case 1: wsManager.viewAll();        break;
                case 2: wsManager.setMaintenance(); break;
                case 3: wsManager.setAvailable();   break;
            }
        } while(c!=0);
    }

    void addCustomer() {
        string nm,cn,ct; int mc;
        cout << "  Name           : "; cin.ignore(); getline(cin,nm);
        cout << "  CNIC           : "; getline(cin,cn);
        cout << "  Contact Number : "; getline(cin,ct);
        cout << "  1. Member (11% discount)  2. New Guest\n  Choice: "; cin >> mc;
        string mtype = (mc==1) ? "Member" : "New Guest";
        customers.push_back(Customer(nextCustomerId++, nm, cn, ct, mtype));
        cout << "  >> Registered! ID:" << (nextCustomerId-1) << " | " << mtype << "\n";
    }

    void viewAllCustomers() const {
        if (customers.empty()) { cout << "  No customers.\n"; return; }
        cout << "\n  ======= ALL CUSTOMERS =======\n";
        for (auto& c : customers)
            cout << "  ID:" << c.getId() << " | " << c.getName()
                 << " | " << c.getMemberType()
                 << " | " << (c.getIsLoggedIn() ? "ACTIVE" : "Idle") << "\n";
        cout << "  =============================\n";
    }

    Customer* findCustomerById(int id) {
        for (auto& c : customers) if (c.getId()==id) return &c;
        return nullptr;
    }

    // ── CHECK IN: only show Idle customers ───────────────────────
    void checkInCustomer() {
        bool any = false;
        cout << "\n  ===== IDLE CUSTOMERS =====\n";
        for (auto& c : customers)
            if (!c.getIsLoggedIn()) {
                cout << "  ID:" << c.getId() << " | " << c.getName()
                     << " | " << c.getMemberType() << " | CNIC:" << c.getCnic() << "\n";
                any = true;
            }
        if (!any) { cout << "  No idle customers.\n"; return; }
        cout << "  ==========================\n";

        cout << "  Enter Customer ID: "; int id; cin >> id;
        Customer* c = findCustomerById(id);
        if (!c)                { cout << "  Not found!\n";       return; }
        if (c->getIsLoggedIn()){ cout << "  Already active!\n";  return; }

        bool peak = isPeakHour();
        if (peak) cout << "\n  ** PEAK HOUR: prices +16% **\n";

        // Select workstation category
        cout << "\n  ===== SELECT SERVICE CATEGORY =====\n";
        cout << "  1. Public/Usage PCs\n";
        cout << "  2. Gaming PCs\n";
        cout << "  3. Designing PCs\n";
        cout << "  4. IT/Software Benches\n";
        cout << "  5. Media/Printing Machines\n";
        cout << "  6. Professional Rooms\n";
        cout << "  0. Cancel\n";
        cout << "  Choice: "; int cc; cin >> cc;
        if (cc==0) return;

        string wsCat;
        switch(cc){
            case 1: wsCat="Public/Usage PCs";        break;
            case 2: wsCat="Gaming PCs";              break;
            case 3: wsCat="Designing PCs";           break;
            case 4: wsCat="IT/Software Benches";     break;
            case 5: wsCat="Media/Printing Machines"; break;
            case 6: wsCat="Professional Rooms";      break;
            default: cout << "  Invalid!\n"; return;
        }

        // Show available workstations
        vector<Workstation*> avail = wsManager.availableInCategory(wsCat);
        if (avail.empty()) { cout << "  No workstations available in " << wsCat << "!\n"; return; }
        cout << "\n  -- Available: " << wsCat << " --\n";
        for (auto* ws : avail) ws->display();
        cout << "  0. Back\n";
        cout << "  Enter Workstation ID: "; int wsId; cin >> wsId;
        if (wsId==0) return;
        Workstation* ws = wsManager.findById(wsId);
        if (!ws||!ws->isAvailable()||ws->category!=wsCat) { cout << "  Invalid!\n"; return; }

        // Pick services
        cout << "\n  ===== SELECT SERVICES =====\n";
        vector<ServiceItem> services = pickServices(peak);

        // Food & drink
        cout << "\n  Food / Drinks?\n  1. Yes   2. No\n  Choice: ";
        int fd; cin >> fd;
        vector<ServiceItem> food;
        if (fd==1) food = pickFood(peak);

        ws->occupy();
        c->startSession(nextSessionId++, wsId, ws->label, peak);
        for (auto& s : services) c->addService(s);
        for (auto& f : food)     c->addFood(f);

        cout << "\n  >> Check In Complete for " << c->getName() << ".\n";
    }

    // ── CHECK OUT: only show Active customers ─────────────────────
    void checkOutCustomer() {
        bool any = false;
        cout << "\n  ===== ACTIVE CUSTOMERS =====\n";
        for (auto& c : customers)
            if (c.getIsLoggedIn()) {
                cout << "  ID:" << c.getId() << " | " << c.getName()
                     << " | WS: " << c.getCurrentWsLabel() << "\n";
                any = true;
            }
        if (!any) { cout << "  No active customers.\n"; return; }
        cout << "  ============================\n";

        cout << "  Enter Customer ID: "; int id; cin >> id;
        Customer* c = findCustomerById(id);
        if (!c||!c->getIsLoggedIn()) { cout << "  Not found or not active!\n"; return; }

        Workstation* ws = wsManager.findById(c->getCurrentWsId());
        if (ws) ws->release();

        string payMethod = pickPaymentMethod();

        // endSession sets isLoggedIn = false internally (KEY FIX)
        Bill bill = c->endSession(nextBillId, payMethod);
        totalRevenue += bill.getGrandTotal();

        // Save to history — customer is now Idle, won't appear in checkIn list
        HistoryRecord rec(c->getId(), c->getName(), c->getCnic(), c->getContactNo(),
                          c->getMemberType(), payMethod,
                          c->getLastSession(), bill,
                          c->getLastServices(), c->getLastFood(), c->getLastPeak());
        history.push_back(rec);
        nextBillId++;

        cout << "  >> Checkout complete. Record saved to History.\n";
    }

    void showHistory() {
        if (history.empty()) { cout << "\n  No history yet.\n"; return; }
        cout << "\n  ========== CHECKOUT HISTORY ==========\n";
        for (int i=0;i<(int)history.size();i++)
            cout << "  [" << (i+1) << "] Bill#" << history[i].billId
                 << " | " << history[i].customerName
                 << " | " << history[i].wsLabel
                 << " | Rs." << fixed << setprecision(2) << history[i].grandTotal
                 << " | " << history[i].checkoutDate << "\n";
        cout << "  ======================================\n";
        cout << "  Select record (0=back): "; int sel; cin >> sel;
        if (sel<=0||sel>(int)history.size()) return;
        HistoryRecord& rec = history[sel-1];
        rec.printInvoice();
        cout << "  1. Regenerate Invoice\n  2. Recheck Details\n  0. Back\n";
        cout << "  Choice: "; int opt; cin >> opt;
        if (opt==1) { cout << "\n  >> Reprinting...\n"; rec.printInvoice(); }
        else if (opt==2) {
            cout << "\n  === RECHECK ===\n";
            cout << "  Bill     : #" << rec.billId << "\n";
            cout << "  Customer : " << rec.customerName << " (ID:" << rec.customerId << ")\n";
            cout << "  CNIC     : " << rec.cnic << "\n";
            cout << "  Contact  : " << rec.contactNo << "\n";
            cout << "  Member   : " << rec.memberType << "\n";
            cout << "  Payment  : " << rec.paymentMethod << "\n";
            cout << "  Station  : " << rec.wsLabel << "\n";
            cout << "  Time     : " << rec.startTime << " -> " << rec.endTime << "\n";
            cout << fixed << setprecision(2);
            cout << "  Duration : " << rec.totalHours << " hrs\n";
            cout << "  Subtotal : Rs." << rec.subtotal << "\n";
            if (rec.discount>0) cout << "  Discount : Rs." << rec.discount << " (11% Member)\n";
            cout << "  Tax      : Rs." << rec.tax << "\n";
            cout << "  Total    : Rs." << rec.grandTotal << "\n";
            cout << "  ===============\n";
        }
    }

    void showRevenueReport() const {
        cout << "\n  ======= REVENUE REPORT =======\n";
        cout << fixed << setprecision(2);
        cout << "  Total Revenue      : Rs." << totalRevenue << "\n";
        cout << "  Total Customers    : " << customers.size() << "\n";
        int active=0; for (auto& c:customers) if (c.getIsLoggedIn()) active++;
        cout << "  Active Sessions    : " << active << "\n";
        cout << "  Completed Sessions : " << history.size() << "\n";
        cout << "  Available Stations : " << wsManager.totalAvailable() << "\n";
        cout << "  ==============================\n";
    }

    void showTopCustomers() const {
        cout << "\n  ======= TOP CUSTOMERS =======\n";
        cout << "  Criteria: Bill > Rs.1199  OR  Duration > 1.5 hrs\n";
        cout << "  -------------------------------------------\n";
        bool found = false;
        for (auto& r : history) {
            if (r.grandTotal > 1199.0f || r.totalHours > 1.5f) {
                cout << "  Bill#" << r.billId
                     << " | " << r.customerName
                     << " | Rs." << fixed << setprecision(2) << r.grandTotal
                     << " | " << r.totalHours << " hrs"
                     << " | " << r.memberType
                     << " | " << r.checkoutDate << "\n";
                found = true;
            }
        }
        if (!found) cout << "  No qualifying customers yet.\n";
        cout << "  ============================\n";
    }
};

// ─── UTILITY ──────────────────────────────────────────
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}
void pauseScreen() {
    cout << "\n  Press Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(),'\n');
    cin.get();
}
void printHeader(string title) {
    clearScreen();
    cout << "  ============================================\n";
    cout << "        CYBERCAFE MANAGEMENT SYSTEM\n";
    cout << "  ============================================\n";
    cout << "   " << title << "\n";
    cout << "  --------------------------------------------\n";
}

// ─── ADMIN MENU ───────────────────────────────────────
void adminMenu(Admin& admin) {
    int choice;
    do {
        printHeader("ADMIN PANEL");
        cout << "  1.  Work Station Management\n";
        cout << "  2.  Customer Management\n";
        cout << "  3.  Check In Customer\n";
        cout << "  4.  Check Out Customer\n";
        cout << "  5.  History\n";
        cout << "  6.  Revenue Report\n";
        cout << "  7.  Top Customers\n";
        cout << "  0.  Logout\n";
        cout << "  --------------------------------------------\n";
        cout << "  Choice: "; cin >> choice;
        switch(choice) {
            case 1: printHeader("WORK STATION MANAGEMENT"); admin.workstationMenu(); pauseScreen(); break;
            case 2: {
                int c; printHeader("CUSTOMER MANAGEMENT");
                cout << "  1. Register New Customer\n  2. View All Customers\n  0. Back\n  Choice: "; cin >> c;
                if (c==1) admin.addCustomer();
                else if (c==2) admin.viewAllCustomers();
                pauseScreen(); break;
            }
            case 3: printHeader("CHECK IN"); admin.checkInCustomer(); pauseScreen(); break;
            case 4: printHeader("CHECK OUT"); admin.checkOutCustomer(); pauseScreen(); break;
            case 5: printHeader("HISTORY"); admin.showHistory(); pauseScreen(); break;
            case 6: printHeader("REVENUE REPORT"); admin.showRevenueReport(); pauseScreen(); break;
            case 7: printHeader("TOP CUSTOMERS"); admin.showTopCustomers(); pauseScreen(); break;
            case 0: cout << "\n  >> Admin logged out.\n"; pauseScreen(); break;
            default: cout << "  Invalid!\n"; pauseScreen();
        }
    } while(choice!=0);
}

// ─── MAIN ─────────────────────────────────────────────
int main() {
    Admin admin(1, "Admin", "admin123");
    int choice;
    do {
        printHeader("MAIN MENU");
        cout << "  1. Admin Login\n";
        cout << "  2. About This System\n";
        cout << "  0. Exit\n";
        cout << "  --------------------------------------------\n";
        cout << "  Choice: "; cin >> choice;
        switch(choice) {
            case 1: {
                printHeader("ADMIN LOGIN");
                string pass; cout << "  Password: "; cin >> pass;
                if (admin.checkPassword(pass)) {
                    cout << "  >> Welcome Admin!\n"; pauseScreen(); adminMenu(admin);
                } else { cout << "  >> Wrong Password!\n"; pauseScreen(); }
                break;
            }
            case 2: {
                printHeader("ABOUT");
                cout << "  System   : CyberCafe Management System\n";
                cout << "  Language : C++ (OOP)\n";
                cout << "  OOP Used : Inheritance, Polymorphism,\n";
                cout << "             Encapsulation, Abstraction,\n";
                cout << "             Dynamic Memory, STL Vectors\n";
                cout << "  Admin Password : admin123\n";
                cout << "  Peak Hours     : 8-10am, 2-4pm, 8-10pm (+16%)\n";
                cout << "  Member Discount: 11% on invoice\n";
                pauseScreen(); break;
            }
            case 0: printHeader("GOODBYE!"); cout << "  Thank you!\n"; break;
            default: cout << "  Invalid!\n"; pauseScreen();
        }
    } while(choice!=0);
    return 0;
}
