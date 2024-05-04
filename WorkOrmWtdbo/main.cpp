#include <iostream>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Postgres.h>

#include <Windows.h>
#pragma execution_character_set("utf-8")

class Book;
class Shop;
class Stock;
class Sale;

class Publisher
{
public:
	//int id; - создается автоматически
	std::string name;
	Wt::Dbo::collection<Wt::Dbo::ptr<Book>> books;

	template<class Action>
	void persist(Action& a)
	{
		//Wt::Dbo::field(a, id, "id");
		Wt::Dbo::field(a, name, "name");
		Wt::Dbo::hasMany(a, books, Wt::Dbo::ManyToOne, "publisher");
	}
};

class Book
{
public:
	// int id;
	std::string title;
	Wt::Dbo::ptr<Publisher> publisher; // publisher_id
	Wt::Dbo::collection<Wt::Dbo::ptr<Stock>> stocks;

	template<class Action>
	void persist(Action& a)
	{
		Wt::Dbo::field(a, title, "title");
		Wt::Dbo::belongsTo(a, publisher, "publisher");
		Wt::Dbo::hasMany(a, stocks, Wt::Dbo::ManyToOne, "book");
	}
};

class Shop
{
public:
	// int id;
	std::string name;
	Wt::Dbo::collection<Wt::Dbo::ptr<Stock>> stocks;

	template<class Action>
	void persist(Action& a)
	{
		Wt::Dbo::field(a, name, "name");
		Wt::Dbo::hasMany(a, stocks, Wt::Dbo::ManyToOne, "shop");
	}
};

class Stock
{
public:
	// int id;
	int count;
	Wt::Dbo::ptr<Book> book; // book_id
	Wt::Dbo::ptr<Shop> shop; // shop_id
	Wt::Dbo::collection<Wt::Dbo::ptr<Sale>> sales;

	template<class Action>
	void persist(Action& a)
	{
		Wt::Dbo::belongsTo(a, book, "book");
		Wt::Dbo::belongsTo(a, shop, "shop");
		Wt::Dbo::field(a, count, "count");
		Wt::Dbo::hasMany(a, sales, Wt::Dbo::ManyToOne, "stock");
	}
};

class Sale
{
public:
	// int id;
	int price;
	std::string data_sale;
	int count;
	Wt::Dbo::ptr<Stock> stock; // stock_id

	template<class Action>
	void persist(Action& a)
	{
		Wt::Dbo::field(a, price, "price");
		Wt::Dbo::field(a, data_sale, "data_sale");
		Wt::Dbo::belongsTo(a, stock, "stock");
		Wt::Dbo::field(a, count, "count");
	}
};


int main()
{
	//setlocale(LC_ALL, "Russian");

	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	setvbuf(stdout, nullptr, _IOFBF, 1000);
	try
	{
		std::string connStr = "host=127.0.0.1"
			" port=5432"
			" dbname=bookstore"
			" user=postgres"
			" password=123";
		auto conn = std::make_unique<Wt::Dbo::backend::Postgres>(connStr);

		Wt::Dbo::Session session;
		session.setConnection(move(conn));

		session.mapClass<Publisher>("publisher");
		session.mapClass<Book>("book");
		session.mapClass<Shop>("shop");
		session.mapClass<Stock>("stock");
		session.mapClass<Sale>("sale");

		session.dropTables();
		session.createTables();

		// Заполнение таблиц и получение данных

		Wt::Dbo::Transaction t{ session };

		// Издатель
		Wt::Dbo::ptr<Publisher> kislDb = session.add(move(std::make_unique<Publisher>(Publisher{ "Kislorod" })));
		Wt::Dbo::ptr<Publisher> eksmoDb = session.add(move(std::make_unique<Publisher>(Publisher{ "Eksmo" })));

		// Книги
		Wt::Dbo::ptr<Book> wingDb = session.add(move(std::make_unique<Book>(Book{ "Четвертое крыло" })));
		Wt::Dbo::ptr<Book> shibariDb = session.add(move(std::make_unique<Book>(Book{ "Канашибари" })));
		wingDb.modify()->publisher = kislDb;
		shibariDb.modify()->publisher = kislDb;

		Wt::Dbo::ptr<Book> insideDb = session.add(move(std::make_unique<Book>(Book{ "Внутри убийцы" })));
		Wt::Dbo::ptr<Book> duneDb = session.add(move(std::make_unique<Book>(Book{ "Дюна" })));
		insideDb.modify()->publisher = eksmoDb;
		duneDb.modify()->publisher = eksmoDb;

		// Магазины
		Wt::Dbo::ptr<Shop> readDb = session.add(move(std::make_unique<Shop>(Shop{ "Читай-город" })));
		Wt::Dbo::ptr<Shop> labDb = session.add(move(std::make_unique<Shop>(Shop{ "Лабиринт" })));

		// Сток
		Wt::Dbo::ptr<Stock> wingStockDb = session.add(move(std::make_unique<Stock>(Stock{ 78 })));
		wingStockDb.modify()->book = wingDb;
		wingStockDb.modify()->shop = readDb;

		Wt::Dbo::ptr<Stock> wingStock2Db = session.add(move(std::make_unique<Stock>(Stock{ 13 })));
		wingStock2Db.modify()->book = wingDb;
		wingStock2Db.modify()->shop = labDb;

		Wt::Dbo::ptr<Stock> insideStockDb = session.add(move(std::make_unique<Stock>(Stock{ 8 })));
		insideStockDb.modify()->book = insideDb;
		insideStockDb.modify()->shop = readDb;

		Wt::Dbo::ptr<Stock> insideStock2Db = session.add(move(std::make_unique<Stock>(Stock{ 53 })));
		insideStock2Db.modify()->book = insideDb;
		insideStock2Db.modify()->shop = labDb;

		// Распродажи
		Wt::Dbo::ptr<Sale> wingSaleDb = session.add(move(std::make_unique<Sale>(Sale{ 699, "2024.05.15", 30})));
		wingSaleDb.modify()->stock = wingStockDb;

		Wt::Dbo::ptr<Sale> insideSaleDb = session.add(move(std::make_unique<Sale>(Sale{ 499, "2024.05.10", 13 })));
		insideSaleDb.modify()->stock = insideStock2Db;

		// Запрос
		std::string namePublisher = "";
		std::cout << "Введите имя издателя:\t";
		std::cin >> namePublisher;
		Wt::Dbo::ptr<Publisher> publIdKislorod = session.find<Publisher>().where("name=?").bind(namePublisher);
		Wt::Dbo::collection<Wt::Dbo::ptr<Book>> booksKislorod = session.find<Book>().where("publisher_id=?").bind(publIdKislorod);

		int i = 0; // счетчик книг
		std::cout << "№\t";
		std::cout << "Книга\t\t";
		std::cout << "Количество\t";
		std::cout << "Магазин\n";
		for (auto book : booksKislorod)
		{
			Wt::Dbo::collection<Wt::Dbo::ptr<Stock>> stocksKislorod = session.find<Stock>().where("book_id=?").bind(book);
			for (auto stock : stocksKislorod)
			{
				Wt::Dbo::ptr<Shop> shopKislorod = session.find<Shop>().where("id=?").bind(stock->shop);

				std::cout << i++ << "\t";
				std::cout << book->title << "\t\t";
				std::cout << stock->count << "\t";
				std::cout << shopKislorod->name << "\t";
				std::cout << "\n";
			}
		}

		if (i == 0)
		{
			std::cout << "Книг данного издателя нет в наличии!\n";
		}

		t.commit();
	}
	catch (const std::runtime_error& err)
	{
		std::cout << err.what() << std::endl;
	}
}