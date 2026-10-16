#pragma once

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

using namespace System;
using namespace System::Drawing;
using namespace System::Globalization;
using namespace System::Windows::Forms;
using namespace MySql::Data::MySqlClient;

public ref class MainForm : public Form
{
private:
    String^ dbConnectionString;
    bool initializing;
    bool seatsLoaded;

    Panel^ page1;
    Panel^ page2;
    Panel^ page3;

    TextBox^ txtName;
    TextBox^ txtPhone;

    ComboBox^ cmbCustomer;
    ComboBox^ cmbMovie;
    ComboBox^ cmbFormat;
    ComboBox^ cmbDate;
    ComboBox^ cmbTime;
    ComboBox^ cmbPayment;

    NumericUpDown^ numTickets;

    Button^ btnBuy;
    Button^ btnConfirm;

    array<Button^>^ seatButtons;
    array<bool>^ bookedSeats;
    array<bool>^ chosenSeats;

    Label^ lblSummary;
    Label^ lblTotal;
    Label^ lblSeatStatus;

    String^ customerName;
    String^ customerPhone;

    double subtotal;
    double discount;
    double paymentFee;
    double total;

    Color darkBg = Color::FromArgb(18, 15, 20);
    Color cardBg = Color::FromArgb(28, 22, 28);
    Color crimsonRed = Color::FromArgb(165, 20, 30);
    Color textWhite = Color::White;
    Color textMuted = Color::FromArgb(180, 175, 180);
    Color seatAvailable = Color::FromArgb(45, 42, 48);
    Color seatBooked = Color::FromArgb(120, 25, 35);
    Color seatSelected = Color::FromArgb(0, 210, 195);

    Label^ AddLabel(
        Control^ parent, String^ text,
        int x, int y, int w, int h)
    {
        Label^ label = gcnew Label();
        label->Text = text;
        label->Location = Point(x, y);
        label->Size = System::Drawing::Size(w, h);
        label->Font = gcnew System::Drawing::Font("Segoe UI", 10);
        label->ForeColor = textMuted;
        label->BackColor = Color::Transparent;
        parent->Controls->Add(label);
        return label;
    }

    TextBox^ AddTextBox(Control^ parent, int x, int y, int w)
    {
        TextBox^ box = gcnew TextBox();
        box->Location = Point(x, y);
        box->Width = w;
        box->Font = gcnew System::Drawing::Font("Segoe UI", 11);
        box->BackColor = Color::FromArgb(38, 32, 40);
        box->ForeColor = textWhite;
        box->BorderStyle = BorderStyle::FixedSingle;
        parent->Controls->Add(box);
        return box;
    }

    ComboBox^ AddCombo(
        Control^ parent, int x, int y,
        int w, array<String^>^ items)
    {
        ComboBox^ combo = gcnew ComboBox();
        combo->Location = Point(x, y);
        combo->Width = w;
        combo->Font = gcnew System::Drawing::Font("Segoe UI", 10);
        combo->BackColor = Color::FromArgb(38, 32, 40);
        combo->ForeColor = textWhite;
        combo->DropDownStyle = ComboBoxStyle::DropDownList;

        for each (String ^ item in items)
            combo->Items->Add(item);

        if (combo->Items->Count > 0)
            combo->SelectedIndex = 0;

        parent->Controls->Add(combo);
        return combo;
    }

    Button^ AddButton(
        Control^ parent, String^ text,
        int x, int y, int w, int h)
    {
        Button^ button = gcnew Button();
        button->Text = text;
        button->Location = Point(x, y);
        button->Size = System::Drawing::Size(w, h);
        button->Font = gcnew System::Drawing::Font(
            "Segoe UI", 11, FontStyle::Bold);
        button->ForeColor = textWhite;
        button->BackColor = crimsonRed;
        button->FlatStyle = FlatStyle::Flat;
        button->FlatAppearance->BorderSize = 0;
        button->Cursor = Cursors::Hand;
        parent->Controls->Add(button);
        return button;
    }

    void ShowPage(int page)
    {
        page1->Visible = page == 1;
        page2->Visible = page == 2;
        page3->Visible = page == 3;

        if (page == 1) page1->BringToFront();
        if (page == 2) page2->BringToFront();
        if (page == 3) page3->BringToFront();
    }

    DateTime GetShowDate()
    {
        return DateTime::ParseExact(
            cmbDate->Text,
            "dd/MM/yyyy",
            CultureInfo::InvariantCulture);
    }

    TimeSpan GetShowTime()
    {
        DateTime parsed = DateTime::ParseExact(
            cmbTime->Text,
            "hh:mm tt",
            CultureInfo::InvariantCulture);

        return parsed.TimeOfDay;
    }

    void AddShowParameters(MySqlCommand^ cmd)
    {
        cmd->Parameters->AddWithValue("@movie", cmbMovie->Text);
        cmd->Parameters->AddWithValue("@format", cmbFormat->Text);
        cmd->Parameters->AddWithValue("@date", GetShowDate());
        cmd->Parameters->AddWithValue("@time", GetShowTime());
    }

    double GetMoviePrice()
    {
        double price = cmbMovie->SelectedIndex == 2 ? 1000.0 : 1500.0;

        if (cmbFormat->SelectedIndex == 1)
            price += 300.0;
        else if (cmbFormat->SelectedIndex == 2)
            price += 200.0;

        return price;
    }

    int SelectedSeatCount()
    {
        int count = 0;

        for each (bool chosen in chosenSeats)
        {
            if (chosen) count++;
        }

        return count;
    }

    String^ SelectedSeatText()
    {
        System::Text::StringBuilder^ text =
            gcnew System::Text::StringBuilder();

        for (int i = 0; i < chosenSeats->Length; i++)
        {
            if (!chosenSeats[i]) continue;

            if (text->Length > 0)
                text->Append(", ");

            text->Append(i + 1);
        }

        return text->ToString();
    }

    void UpdateSeatButtons()
    {
        int available = 0;

        for (int i = 0; i < seatButtons->Length; i++)
        {
            Button^ seat = seatButtons[i];
            seat->Enabled = seatsLoaded && !bookedSeats[i];

            if (bookedSeats[i])
            {
                seat->BackColor = seatBooked;
                seat->ForeColor = textWhite;
            }
            else if (chosenSeats[i])
            {
                seat->BackColor = seatSelected;
                seat->ForeColor = Color::Black;
            }
            else
            {
                seat->BackColor = seatAvailable;
                seat->ForeColor = textWhite;
            }

            if (!bookedSeats[i]) available++;
        }

        btnBuy->Enabled = seatsLoaded;

        lblSeatStatus->Text = seatsLoaded
            ? String::Format(
                "Available: {0}    Selected: {1}",
                available, SelectedSeatCount())
            : "Seat availability could not be loaded.";
    }

    bool LoadBookedSeats(bool clearSelection)
    {
        seatsLoaded = false;

        if (clearSelection)
            Array::Clear(chosenSeats, 0, chosenSeats->Length);

        UpdateSeatButtons();

        MySqlConnection^ conn =
            gcnew MySqlConnection(dbConnectionString);

        MySqlCommand^ cmd = nullptr;
        MySqlDataReader^ reader = nullptr;

        try
        {
            conn->Open();
            cmd = conn->CreateCommand();

            cmd->CommandText =
                "SELECT seat_number FROM booked_seats "
                "WHERE movie_name = @movie "
                "AND movie_format = @format "
                "AND show_date = @date "
                "AND show_time = @time";

            AddShowParameters(cmd);

            array<bool>^ loadedSeats = gcnew array<bool>(20);
            reader = cmd->ExecuteReader();

            while (reader->Read())
            {
                int seatNumber = reader->GetInt32(0);

                if (seatNumber >= 1 && seatNumber <= 20)
                    loadedSeats[seatNumber - 1] = true;
            }

            bookedSeats = loadedSeats;

            for (int i = 0; i < chosenSeats->Length; i++)
            {
                if (bookedSeats[i])
                    chosenSeats[i] = false;
            }

            seatsLoaded = true;
            UpdateSeatButtons();
            return true;
        }
        catch (Exception^ ex)
        {
            UpdateSeatButtons();
            MessageBox::Show(
                ex->Message,
                "Could not load seats",
                MessageBoxButtons::OK,
                MessageBoxIcon::Error);

            return false;
        }
        finally
        {
            if (reader != nullptr) delete reader;
            if (cmd != nullptr) delete cmd;
            delete conn;
        }
    }

    void CalculateTotals()
    {
        int wanted = Convert::ToInt32(numTickets->Value);

        subtotal = Math::Round(GetMoviePrice() * wanted, 2);
        discount = cmbCustomer->SelectedIndex == 1
            ? Math::Round(subtotal * 0.10, 2)
            : 0.0;

        total = subtotal - discount;

        double feeRate = cmbPayment->SelectedIndex == 1 ? 0.01 :
            cmbPayment->SelectedIndex == 2 ? 0.02 : 0.0;

        paymentFee = Math::Round(total * feeRate, 2);
    }

    void UpdatePaymentTotal()
    {
        CalculateTotals();

        lblTotal->Text = String::Format(
            "Booking: Rs. {0:F2}   Fee: Rs. {1:F2}\n"
            "Amount due: Rs. {2:F2}",
            total, paymentFee, total + paymentFee);
    }

    void UpdateSummary()
    {
        CalculateTotals();

        lblSummary->Text =
            "CUSTOMER: " + customerName +
            "\nPhone: " + customerPhone +
            "\nCustomer type: " + cmbCustomer->Text +
            "\nMovie: " + cmbMovie->Text +
            "\nFormat: " + cmbFormat->Text +
            "\nDate: " + cmbDate->Text +
            "\nTime: " + cmbTime->Text +
            "\nSeats: " + SelectedSeatText() +
            "\nTickets: " + SelectedSeatCount().ToString() +
            "\nTicket price: Rs. " + GetMoviePrice().ToString("F2") +
            "\nSubtotal: Rs. " + subtotal.ToString("F2") +
            "\nDiscount: Rs. " + discount.ToString("F2");

        UpdatePaymentTotal();
    }

    int SaveBooking()
    {
        MySqlConnection^ conn =
            gcnew MySqlConnection(dbConnectionString);

        MySqlTransaction^ transaction = nullptr;
        MySqlCommand^ cmd = nullptr;

        try
        {
            conn->Open();
            transaction = conn->BeginTransaction();

            cmd = conn->CreateCommand();
            cmd->Transaction = transaction;

            cmd->CommandText =
                "INSERT INTO customers "
                "(full_name, phone, customer_type) "
                "VALUES (@name, @phone, @type)";

            cmd->Parameters->AddWithValue("@name", customerName);
            cmd->Parameters->AddWithValue("@phone", customerPhone);
            cmd->Parameters->AddWithValue(
                "@type",
                cmbCustomer->SelectedIndex == 1
                ? "Premium" : "Regular");

            cmd->ExecuteNonQuery();
            int savedCustomerID =
                Convert::ToInt32(cmd->LastInsertedId);

            cmd->Parameters->Clear();
            cmd->CommandText =
                "INSERT INTO bookings "
                "(customer_id, movie_name, movie_format, show_date, "
                "show_time, ticket_count, ticket_price, subtotal, "
                "discount, payment_method, payment_fee, total_paid) "
                "VALUES (@customer, @movie, @format, @date, @time, "
                "@count, @price, @subtotal, @discount, @payment, "
                "@fee, @paid)";

            AddShowParameters(cmd);

            String^ paymentMethod =
                cmbPayment->SelectedIndex == 0 ? "Cash" :
                cmbPayment->SelectedIndex == 1 ? "Card" : "Online";

            cmd->Parameters->AddWithValue("@customer", savedCustomerID);
            cmd->Parameters->AddWithValue("@count", SelectedSeatCount());
            cmd->Parameters->AddWithValue(
                "@price", Convert::ToDecimal(GetMoviePrice()));
            cmd->Parameters->AddWithValue(
                "@subtotal", Convert::ToDecimal(subtotal));
            cmd->Parameters->AddWithValue(
                "@discount", Convert::ToDecimal(discount));
            cmd->Parameters->AddWithValue("@payment", paymentMethod);
            cmd->Parameters->AddWithValue(
                "@fee", Convert::ToDecimal(paymentFee));
            cmd->Parameters->AddWithValue(
                "@paid", Convert::ToDecimal(total + paymentFee));

            cmd->ExecuteNonQuery();
            int bookingID = Convert::ToInt32(cmd->LastInsertedId);

            cmd->Parameters->Clear();
            cmd->CommandText =
                "INSERT INTO booked_seats "
                "(booking_id, movie_name, movie_format, "
                "show_date, show_time, seat_number) "
                "VALUES (@booking, @movie, @format, @date, @time, @seat)";

            AddShowParameters(cmd);
            cmd->Parameters->AddWithValue("@booking", bookingID);
            cmd->Parameters->AddWithValue("@seat", 0);

            for (int i = 0; i < chosenSeats->Length; i++)
            {
                if (!chosenSeats[i]) continue;

                cmd->Parameters["@seat"]->Value = i + 1;
                cmd->ExecuteNonQuery();
            }

            transaction->Commit();
            return bookingID;
        }
        catch (Exception^)
        {
            if (transaction != nullptr)
            {
                try
                {
                    transaction->Rollback();
                }
                catch (Exception^)
                {
                    // Keep the original save error.
                }
            }

            throw;
        }
        finally
        {
            if (cmd != nullptr) delete cmd;
            if (transaction != nullptr) delete transaction;
            delete conn;
        }
    }

    void ResetBooking()
    {
        initializing = true;

        txtName->Clear();
        txtPhone->Clear();

        cmbCustomer->SelectedIndex = 0;
        cmbMovie->SelectedIndex = 0;
        cmbFormat->SelectedIndex = 0;
        cmbDate->SelectedIndex = 0;
        cmbTime->SelectedIndex = 0;
        cmbPayment->SelectedIndex = 0;

        numTickets->Value = Decimal(1);
        customerName = String::Empty;
        customerPhone = String::Empty;
        subtotal = discount = paymentFee = total = 0.0;

        Array::Clear(chosenSeats, 0, chosenSeats->Length);
        Array::Clear(bookedSeats, 0, bookedSeats->Length);

        seatsLoaded = false;
        lblSummary->Text = String::Empty;
        lblTotal->Text = String::Empty;

        initializing = false;
        UpdateSeatButtons();
        ShowPage(1);
        txtName->Focus();
    }

    void OnNext1(Object^ sender, EventArgs^ e)
    {
        customerName = txtName->Text->Trim();
        customerPhone = txtPhone->Text->Trim();

        if (String::IsNullOrWhiteSpace(customerName))
        {
            MessageBox::Show("Enter customer name.");
            txtName->Focus();
            return;
        }

        if (customerPhone->Length < 7 || customerPhone->Length > 15)
        {
            MessageBox::Show("Phone number must contain 7-15 digits.");
            txtPhone->Focus();
            return;
        }

        for each (Char ch in customerPhone)
        {
            if (ch < L'0' || ch > L'9')
            {
                MessageBox::Show("Phone number must contain digits only.");
                txtPhone->Focus();
                return;
            }
        }

        ShowPage(2);
        LoadBookedSeats(true);
    }

    void OnShowChanged(Object^ sender, EventArgs^ e)
    {
        if (initializing) return;
        LoadBookedSeats(true);
    }

    void OnRefreshSeats(Object^ sender, EventArgs^ e)
    {
        LoadBookedSeats(false);
    }

    void OnSeatClick(Object^ sender, EventArgs^ e)
    {
        if (!seatsLoaded) return;

        Button^ button = safe_cast<Button^>(sender);
        int index = Convert::ToInt32(button->Tag);

        if (bookedSeats[index]) return;

        if (!chosenSeats[index] &&
            SelectedSeatCount() >= Convert::ToInt32(numTickets->Value))
        {
            MessageBox::Show(
                "Increase the ticket count before selecting more seats.");
            return;
        }

        chosenSeats[index] = !chosenSeats[index];
        UpdateSeatButtons();
    }

    void OnNext2(Object^ sender, EventArgs^ e)
    {
        if (!LoadBookedSeats(false)) return;

        int wanted = Convert::ToInt32(numTickets->Value);

        if (SelectedSeatCount() != wanted)
        {
            MessageBox::Show(
                "Select exactly the requested number of available seats.");
            return;
        }

        UpdateSummary();
        ShowPage(3);
    }

    void OnBack2(Object^ sender, EventArgs^ e)
    {
        ShowPage(1);
    }

    void OnBack3(Object^ sender, EventArgs^ e)
    {
        ShowPage(2);
        LoadBookedSeats(false);
    }

    void OnPaymentChanged(Object^ sender, EventArgs^ e)
    {
        if (initializing) return;
        UpdatePaymentTotal();
    }

    void OnConfirm(Object^ sender, EventArgs^ e)
    {
        if (!seatsLoaded ||
            SelectedSeatCount() != Convert::ToInt32(numTickets->Value))
        {
            MessageBox::Show("Go back and check your seat selection.");
            ShowPage(2);
            LoadBookedSeats(false);
            return;
        }

        CalculateTotals();
        String^ seatText = SelectedSeatText();

        int bookingID = 0;
        btnConfirm->Enabled = false;

        try
        {
            bookingID = SaveBooking();
        }
        catch (MySqlException^ ex)
        {
            if (ex->Number == 1062)
            {
                MessageBox::Show(
                    "A selected seat was booked by another customer. "
                    "Please choose another available seat.",
                    "Seat unavailable");

                ShowPage(2);
                LoadBookedSeats(false);
            }
            else
            {
                MessageBox::Show(ex->Message, "Booking not confirmed");
            }

            return;
        }
        catch (Exception^ ex)
        {
            MessageBox::Show(ex->Message, "Booking not confirmed");
            return;
        }
        finally
        {
            btnConfirm->Enabled = true;
        }

        MessageBox::Show(
            "BOOKING CONFIRMED!\n\n"
            "Booking ID: " + bookingID.ToString() +
            "\nCustomer: " + customerName +
            "\nPhone: " + customerPhone +
            "\nMovie: " + cmbMovie->Text +
            "\nFormat: " + cmbFormat->Text +
            "\nDate: " + cmbDate->Text +
            "\nTime: " + cmbTime->Text +
            "\nSeats: " + seatText +
            "\nPayment: " + cmbPayment->Text +
            "\nBooking total: Rs. " + total.ToString("F2") +
            "\nPayment fee: Rs. " + paymentFee.ToString("F2") +
            "\nAmount paid: Rs. " + (total + paymentFee).ToString("F2"),
            "Booking Confirmation",
            MessageBoxButtons::OK,
            MessageBoxIcon::Information);

        ResetBooking();
    }

public:
    MainForm()
    {
        initializing = true;
        seatsLoaded = false;
        subtotal = discount = paymentFee = total = 0.0;

        bookedSeats = gcnew array<bool>(20);
        chosenSeats = gcnew array<bool>(20);
        seatButtons = gcnew array<Button^>(20);

        MySqlConnectionStringBuilder^ settings =
            gcnew MySqlConnectionStringBuilder();

        settings->Server = "127.0.0.1";
        settings->Port = 3306;
        settings->Database = "movie_ticket_booking";
        settings->UserID = "movie_app";
        settings->Password = "root";

        dbConnectionString = settings->ConnectionString;

        Text = "Movie Ticket Booking System";
        ClientSize = System::Drawing::Size(900, 700);
        MinimumSize = System::Drawing::Size(920, 740);
        StartPosition = FormStartPosition::CenterScreen;
        BackColor = darkBg;
        AutoScaleMode = System::Windows::Forms::AutoScaleMode::Dpi;

        // PAGE 1: Customer and movie.

        page1 = gcnew Panel();
        page1->Dock = DockStyle::Fill;
        page1->BackColor = darkBg;
        page1->AutoScroll = true;
        Controls->Add(page1);

        Label^ title1 = AddLabel(
            page1, "MOVIE TICKET BOOKING", 220, 30, 540, 45);
        title1->Font = gcnew System::Drawing::Font(
            "Segoe UI", 20, FontStyle::Bold);
        title1->ForeColor = textWhite;

        AddLabel(page1, "Customer name:", 120, 110, 150, 30);
        txtName = AddTextBox(page1, 300, 105, 320);
        txtName->MaxLength = 150;

        AddLabel(page1, "Phone number:", 120, 170, 150, 30);
        txtPhone = AddTextBox(page1, 300, 165, 320);
        txtPhone->MaxLength = 15;

        AddLabel(page1, "Customer type:", 120, 230, 150, 30);
        cmbCustomer = AddCombo(
            page1, 300, 225, 320,
            gcnew array<String^>{
            "Regular", "Premium (10% discount)"
        });

        AddLabel(page1, "Movie:", 120, 290, 150, 30);
        cmbMovie = AddCombo(
            page1, 300, 285, 320,
            gcnew array<String^>{
            "Avengers", "Avatar", "Interstellar"
        });

        AddLabel(page1, "Format:", 120, 350, 150, 30);
        cmbFormat = AddCombo(
            page1, 300, 345, 320,
            gcnew array<String^>{"2D", "3D", "IMAX"});

        Button^ next1 = AddButton(
            page1, "Reservation", 350, 440, 180, 45);
        next1->Click += gcnew EventHandler(this, &MainForm::OnNext1);

        // PAGE 2: Show and seats.

        page2 = gcnew Panel();
        page2->Dock = DockStyle::Fill;
        page2->BackColor = darkBg;
        page2->AutoScroll = true;
        Controls->Add(page2);

        Label^ title2 = AddLabel(
            page2, "Choose your seats", 300, 25, 350, 40);
        title2->Font = gcnew System::Drawing::Font(
            "Segoe UI", 18, FontStyle::Bold);
        title2->ForeColor = textWhite;

        array<String^>^ dates = gcnew array<String^>(14);

        for (int i = 0; i < dates->Length; i++)
        {
            dates[i] = DateTime::Today.AddDays(i).ToString(
                "dd/MM/yyyy", CultureInfo::InvariantCulture);
        }

        AddLabel(page2, "Date:", 120, 80, 60, 30);
        cmbDate = AddCombo(page2, 180, 75, 170, dates);

        AddLabel(page2, "Time:", 390, 80, 60, 30);
        cmbTime = AddCombo(
            page2, 450, 75, 170,
            gcnew array<String^>{
            "10:00 AM", "02:00 PM", "07:00 PM"
        });

        Label^ availableLegend = AddLabel(
            page2, "Available", 180, 125, 110, 25);
        availableLegend->BackColor = seatAvailable;
        availableLegend->ForeColor = textWhite;

        Label^ reservedLegend = AddLabel(
            page2, "Reserved", 340, 125, 110, 25);
        reservedLegend->BackColor = seatBooked;
        reservedLegend->ForeColor = textWhite;

        Label^ selectedLegend = AddLabel(
            page2, "Selected", 500, 125, 110, 25);
        selectedLegend->BackColor = seatSelected;
        selectedLegend->ForeColor = Color::Black;

        for (int i = 0; i < 20; i++)
        {
            int row = i / 5;
            int col = i % 5;

            Button^ seat = AddButton(
                page2, (i + 1).ToString(),
                150 + col * 110, 170 + row * 52, 90, 38);

            seat->Tag = i;
            seat->BackColor = seatAvailable;
            seat->Font = gcnew System::Drawing::Font(
                "Segoe UI", 9, FontStyle::Bold);
            seat->Click += gcnew EventHandler(
                this, &MainForm::OnSeatClick);

            seatButtons[i] = seat;
        }

        lblSeatStatus = AddLabel(
            page2, "", 150, 380, 600, 25);

        AddLabel(page2, "Tickets:", 150, 420, 70, 30);

        numTickets = gcnew NumericUpDown();
        numTickets->Location = Point(230, 418);
        numTickets->Width = 100;
        numTickets->Minimum = Decimal(1);
        numTickets->Maximum = Decimal(20);
        numTickets->Value = Decimal(1);
        numTickets->Font = gcnew System::Drawing::Font("Segoe UI", 10);
        numTickets->BackColor = Color::FromArgb(38, 32, 40);
        numTickets->ForeColor = textWhite;
        page2->Controls->Add(numTickets);

        Button^ refresh = AddButton(
            page2, "Refresh seats", 450, 415, 180, 38);
        refresh->Click += gcnew EventHandler(
            this, &MainForm::OnRefreshSeats);

        Button^ back2 = AddButton(
            page2, "Back", 230, 490, 130, 45);
        back2->Click += gcnew EventHandler(this, &MainForm::OnBack2);

        btnBuy = AddButton(page2, "Buy", 410, 490, 130, 45);
        btnBuy->Click += gcnew EventHandler(this, &MainForm::OnNext2);

        // PAGE 3: Summary and confirmation.

        page3 = gcnew Panel();
        page3->Dock = DockStyle::Fill;
        page3->BackColor = darkBg;
        page3->AutoScroll = true;
        Controls->Add(page3);

        Label^ title3 = AddLabel(
            page3, "Mobile Ticket", 330, 25, 300, 40);
        title3->Font = gcnew System::Drawing::Font(
            "Segoe UI", 18, FontStyle::Bold);
        title3->ForeColor = textWhite;

        lblSummary = AddLabel(page3, "", 120, 80, 660, 290);
        lblSummary->Font = gcnew System::Drawing::Font("Consolas", 11);
        lblSummary->ForeColor = textWhite;
        lblSummary->BackColor = cardBg;

        lblTotal = AddLabel(page3, "", 120, 385, 660, 70);
        lblTotal->Font = gcnew System::Drawing::Font(
            "Segoe UI", 13, FontStyle::Bold);
        lblTotal->ForeColor = textWhite;

        AddLabel(page3, "Payment method:", 120, 475, 160, 30);
        cmbPayment = AddCombo(
            page3, 290, 470, 270,
            gcnew array<String^>{
            "Cash (0% fee)",
                "Card (1% fee)",
                "Online (2% fee)"
        });

        Button^ back3 = AddButton(
            page3, "Back", 240, 540, 140, 45);
        back3->Click += gcnew EventHandler(this, &MainForm::OnBack3);

        btnConfirm = AddButton(
            page3, "Confirm Booking", 410, 540, 200, 45);
        btnConfirm->Click += gcnew EventHandler(
            this, &MainForm::OnConfirm);

        cmbDate->SelectedIndexChanged += gcnew EventHandler(
            this, &MainForm::OnShowChanged);
        cmbTime->SelectedIndexChanged += gcnew EventHandler(
            this, &MainForm::OnShowChanged);
        cmbPayment->SelectedIndexChanged += gcnew EventHandler(
            this, &MainForm::OnPaymentChanged);

        initializing = false;
        UpdateSeatButtons();
        ShowPage(1);
    }
};