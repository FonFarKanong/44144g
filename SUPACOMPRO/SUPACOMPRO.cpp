#include <SFML/Graphics.hpp>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <string>
#include <iostream>

using namespace std;

//ขนาดหน้าจอ
const int WIDTH = 1280;
const int HEIGHT = 800;

//มีไว้เพื่อป้องกันชื่อชนกัน
enum class GameState {
    MENU,
    COUNTDOWN,
    PLAYING,
    GAME_OVER,
	HOW_TO_PLAY
};

// ------------------------- MainMenu Class -------------------------
class MainMenu {
private:
	sf::Font& font;// ฟอนต์ที่ใช้ในเมนู
	sf::Text titleRobo, titleBie, startText, howToPlayText;// ข้อความต่างๆในเมนู
	sf::RectangleShape startButton, howToPlayButton;// ปุ่มต่างๆในเมนู
	sf::Texture backgroundTexture;// พื้นหลังของเมนู
	sf::Sprite backgroundSprite;// สไปรต์สำหรับพื้นหลัง

public:
    MainMenu(sf::Font& f) : font(f) {
        if (!backgroundTexture.loadFromFile("background_menu.png")) {
            cerr << "Failed to load menu background!" << endl;
		}//ไว้เช็คเฉยๆว่า Background โหลดได้ไหม
		backgroundSprite.setTexture(backgroundTexture);//โหลด Background
        backgroundSprite.setScale(
            (float)WIDTH / backgroundTexture.getSize().x,
            (float)HEIGHT / backgroundTexture.getSize().y
		);//ปรับขนาด Background ให้เต็มจอ

        titleRobo.setFont(font);//ล็อคฟ้อน
        titleRobo.setString("ROBO");//ตั้งให้ขึ้น ROBO
		titleRobo.setCharacterSize(50);//ขนาดตัวอักษร
		titleRobo.setFillColor(sf::Color::White);//สีตัวอักษร
        titleRobo.setPosition(WIDTH / 2 - 60, 150);  // ปรับให้ ROBO อยู่ซ้ายหน่อย

        titleBie.setFont(font);
        titleBie.setString("BIE");
        titleBie.setCharacterSize(50);
        titleBie.setFillColor(sf::Color::Red);
        titleBie.setPosition(WIDTH / 2 + 50, 150);  // ปรับให้ BIE อยู่ขวาหน่อย


        startButton.setSize({ 200, 70 });
        startButton.setPosition((WIDTH - 200) / 2, 300);
        startButton.setFillColor(sf::Color::Black);
        startButton.setOutlineColor(sf::Color::Green);
		startButton.setOutlineThickness(3);//ปรับความหนาของเส้นขอบปุ่ม

        startText.setFont(font);
        startText.setString("Start");
        startText.setCharacterSize(40);
        startText.setFillColor(sf::Color::Green);
		startText.setOrigin(startText.getLocalBounds().width / 2, startText.getLocalBounds().height / 2);//ปรับตำแหน่งให้ตรงกลาง
        startText.setPosition(WIDTH / 2, 310);

        howToPlayButton.setSize({ 200, 70 });
        howToPlayButton.setPosition((WIDTH - 200) / 2, 400);
        howToPlayButton.setFillColor(sf::Color::Black);
        howToPlayButton.setOutlineColor(sf::Color::White);
        howToPlayButton.setOutlineThickness(3);

        howToPlayText.setFont(font);
        howToPlayText.setString("How to Play");
        howToPlayText.setCharacterSize(30);
        howToPlayText.setFillColor(sf::Color::White);
        howToPlayText.setOrigin(howToPlayText.getLocalBounds().width / 2, howToPlayText.getLocalBounds().height / 2);
        howToPlayText.setPosition(WIDTH / 2, 410);
    }

	bool handleClick(sf::Vector2i mousePos) {//ฟังก์ชันนี้จะเช็คว่าปุ่ม Start ถูกคลิกหรือไม่
        if (startButton.getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y)) {
            return true;
        }
        return false;
    }

	void draw(sf::RenderWindow& window) {//ฟังก์ชันนี้จะวาดทุกอย่างที่อยู่ในเมนู
		window.draw(backgroundSprite);// วาดพื้นหลัง
		window.draw(titleRobo);// วาดชื่อ ROBO
		window.draw(titleBie);// วาดชื่อ BIE
		window.draw(startButton);// วาดปุ่ม Start
		window.draw(startText);// วาดข้อความบนปุ่ม Start
		window.draw(howToPlayButton);// วาดปุ่ม How to Play
		window.draw(howToPlayText);// วาดข้อความบนปุ่ม How to Play
    }
};

// ------------------------- GameManager Class -------------------------
class GameManager {//คลาสนี้จะจัดการเกมทั้งหมด
private:
	sf::RenderWindow& window;// หน้าต่างที่ใช้วาด
	sf::Font& font;// ฟอนต์ที่ใช้ในเกม
    float virusSpawnInterval = 1000.f; // เวลาเริ่มต้นหน่วยมิลลิวินาที
	sf::Clock totalTimeClock;// เวลาใช้สำหรับคำนวณเวลาทั้งหมดที่เล่นไป

	sf::Texture robot1, robot2;// โหลดรูปหุ่นยนต์ 2 รูป
	sf::Sprite robot;// สร้างสไปรต์สำหรับหุ่นยนต์
	bool useFirstFrame = true;// ใช้เพื่อสลับระหว่างรูปหุ่นยนต์ 1 และ 2

    sf::Texture virus1, virus2;
    struct Virus {
        sf::Sprite sprite;
        bool useFirstFrame = true;
        sf::Clock animClock;
    };
	std::vector<Virus> viruses;// สร้างเวกเตอร์สำหรับเก็บไวรัส

	struct Bullet {// โครงสร้างสำหรับกระสุน
		sf::CircleShape shape;// รูปร่างของกระสุน
		float speed = 10.f;// ความเร็วของกระสุน
    };
    std::vector<Bullet> bullets;

	sf::Clock animationClock, virusClock, shootClock, reloadClock;// เวลาสำหรับจัดการเวลาในการเคลื่อนไหวและการเกิดไวรัส
	int hp = 3;// จำนวนชีวิตของหุ่นยนต์
	int score = 0;// คะแนนของผู้เล่น
	int bulletsLeft = 12;// จำนวนกระสุนที่เหลือ
	bool isGameOver = false;// สถานะของเกมว่าเป็นเกมจบหรือยังไม่จบ

	sf::Text scoreText, gameOverText;// ข้อความสำหรับแสดงคะแนนและข้อความเกมจบ
	sf::RectangleShape backToMenuBtn;// ปุ่มกลับไปที่เมนูหลัก
	sf::Text backToMenuText;// ข้อความบนปุ่มกลับไปที่เมนูหลัก
    sf::Text reloadText;// ข้อความรีโหลด
	bool isReloading = false;// สถานะการรีโหลดกระสุน


	sf::RectangleShape shieldZone;// เขตที่ห้ามไวรัสเข้ามา

public:
	GameManager(sf::RenderWindow& win, sf::Font& f) : window(win), font(f) {
		robot1.loadFromFile("robot1.png");// โหลดรูปหุ่นยนต์ 1
		robot2.loadFromFile("robot2.png");// โหลดรูปหุ่นยนต์ 2
		robot.setTexture(robot1);// ตั้งค่าให้หุ่นยนต์ใช้รูปแรก
		robot.setPosition(50, HEIGHT / 2);// ตั้งตำแหน่งเริ่มต้นของหุ่นยนต์
		robot.setScale(1.5f, 1.5f);// ปรับขนาดหุ่นยนต์ให้ใหญ่ขึ้น

		virus1.loadFromFile("virus1.png");// โหลดรูปไวรัส 1
		virus2.loadFromFile("virus2.png");// โหลดรูปไวรัส 2

		scoreText.setFont(font);// ตั้งค่าให้ข้อความคะแนนใช้ฟอนต์ที่โหลดมา
		scoreText.setCharacterSize(24);// ขนาดตัวอักษรของคะแนน
		scoreText.setFillColor(sf::Color::White);// สีตัวอักษรของคะแนน
		scoreText.setPosition(10, 10);// ตำแหน่งของข้อความคะแนน

        reloadText.setFont(font);
        reloadText.setCharacterSize(24);
        reloadText.setFillColor(sf::Color::Cyan);
        reloadText.setPosition(10, 40);
        reloadText.setString("");

        gameOverText.setFont(font);
        gameOverText.setCharacterSize(48);
        gameOverText.setFillColor(sf::Color::Red);
        gameOverText.setString("GAME OVER");
        gameOverText.setOrigin(gameOverText.getLocalBounds().width / 2, gameOverText.getLocalBounds().height / 2);
        gameOverText.setPosition(WIDTH / 2, HEIGHT / 2 - 100);

        backToMenuBtn.setSize({ 200, 60 });
        backToMenuBtn.setFillColor(sf::Color::Black);
        backToMenuBtn.setOutlineColor(sf::Color::White);
        backToMenuBtn.setOutlineThickness(3);
        backToMenuBtn.setPosition(WIDTH / 2 - 100, HEIGHT / 2 + 50);

        backToMenuText.setFont(font);
        backToMenuText.setString("Main Menu");
        backToMenuText.setCharacterSize(28);
        backToMenuText.setFillColor(sf::Color::White);
        backToMenuText.setOrigin(backToMenuText.getLocalBounds().width / 2, backToMenuText.getLocalBounds().height / 2);
        backToMenuText.setPosition(WIDTH / 2, HEIGHT / 2 + 60);

        shieldZone.setSize({ 5, HEIGHT });
        shieldZone.setFillColor(sf::Color::White);
    }

	void reset() {// ฟังก์ชันนี้จะรีเซ็ตค่าทุกอย่างเมื่อเริ่มเกมใหม่
		totalTimeClock.restart();// รีเซ็ตเวลา(ปกติทุกๆวินาทีที่ผ่านไปซอมบี้จะยิ่งเกิดเร็วขึ้น)
		bullets.clear();// ล้างกระสุนทั้งหมด
		viruses.clear();// ล้างไวรัสทั้งหมด
		hp = 3;// รีเซ็ตจำนวน Hp
		score = 0;// รีเซ็ตคะแนน
        bulletsLeft = 12; // รีเซ็ตจำนวนกระสุน
        isGameOver = false;// รีเซ้ต Game over ให้ไม่เข้าเงื่อนไข
        robot.setPosition(50, HEIGHT / 2);
    }

	void handleEvent(sf::Event& event, GameState& state) {// ฟังก์ชันนี้จะจัดการกับเหตุการณ์ต่างๆ ที่เกิดขึ้นในเกม
		if (isGameOver && event.type == sf::Event::MouseButtonPressed) {// ถ้าเกมจบแล้วและมีการคลิกเมาส์
			auto mp = sf::Mouse::getPosition(window);// รับตำแหน่งเมาส์
			if (backToMenuBtn.getGlobalBounds().contains((float)mp.x, (float)mp.y)) {// ถ้าคลิกที่ปุ่มกลับไปที่เมนู
                reset();
                state = GameState::MENU;
            }
        }

		if (!isGameOver && event.type == sf::Event::KeyPressed) {// ถ้าเกมยังไม่จบและมีการกดปุ่ม
			if (event.key.code == sf::Keyboard::R && !isReloading) {// ถ้ากดปุ่ม R และไม่ได้กำลังรีโหลด
				isReloading = true;// ตั้งค่าสถานะการรีโหลดเป็นจริง
				reloadClock.restart();// รีเซ็ตนาฬิการีโหลด
				reloadText.setString("Reloading... ");// แสดงข้อความรีโหลด
            }
        }
    }

	void update(GameState& state) {// ฟังก์ชันนี้จะอัพเดทสถานะของเกมทุกครั้งที่เรียกใช้
		if (isGameOver) return;// ถ้าเกมจบแล้วไม่ต้องทำอะไร

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && robot.getPosition().y > 0)// ถ้ากดปุ่ม W และหุ่นยนต์ไม่อยู่ที่ขอบบน
			robot.move(0, -5);// เคลื่อนที่ขึ้น
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) && robot.getPosition().y + robot.getGlobalBounds().height < HEIGHT + 10)// ถ้ากดปุ่ม S และหุ่นยนต์ไม่อยู่ที่ขอบล่าง + เพื่อไม่ให้ไวรัสออกนอกจอ
			robot.move(0, 5);// เคลื่อนที่ลง

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && shootClock.getElapsedTime().asMilliseconds() > 300 && bulletsLeft > 0) {// ถ้ากดปุ่ม Space และเวลาที่ผ่านไปตั้งแต่ยิงมากกว่า 300 มิลลิวินาที และยังมีกระสุนเหลือ
			Bullet b;// สร้างกระสุนใหม่
			b.shape.setRadius(5);// ตั้งรัศมีของกระสุน
			b.shape.setFillColor(sf::Color::Yellow);// ตั้งสีของกระสุน
			b.shape.setPosition(robot.getPosition().x + 40, robot.getPosition().y + 15);// ตั้งตำแหน่งของกระสุนให้เริ่มจากหุ่นยนต์
			bullets.push_back(b);// เพิ่มกระสุนเข้าไปในเวกเตอร์
			bulletsLeft--;// ลดจำนวนกระสุนที่เหลือ
			shootClock.restart();// รีเซ็ตเวลายิงกระสุน
        }

        for (auto& b : bullets) b.shape.move(b.speed, 0);
        bullets.erase(remove_if(bullets.begin(), bullets.end(), [](Bullet& b) {
            return b.shape.getPosition().x > WIDTH;
            }), bullets.end());

        // คำนวณเวลาที่เล่นไปทั้งหมดเป็นวินาที
        float totalSeconds = totalTimeClock.getElapsedTime().asSeconds();

        // ลดช่วงเวลาการในเกิดไวรัสลงเรื่อย ๆ (ลด 20 ms ต่อ 1 วินาที แต่ไม่ให้ต่ำกว่า 200 ms
        virusSpawnInterval = 1500.f - totalSeconds * 50.f;
        if (virusSpawnInterval < 200.f) virusSpawnInterval = 200.f;

        // ใช้ virusSpawnInterval แทนค่า 2000 เดิม
		if (virusClock.getElapsedTime().asMilliseconds() > virusSpawnInterval) {// ถ้าเวลาที่ผ่านไปตั้งแต่เกิดไวรัสมากกว่า virusSpawnInterval
			Virus v;// สร้างไวรัสใหม่
			v.sprite.setTexture(virus1);// ตั้งค่าให้ไวรัสใช้รูปแรก
			v.sprite.setScale(0.2f, 0.2f);// ปรับขนาดไวรัสให้เล็กลง
			v.sprite.setPosition(WIDTH, rand() % (HEIGHT - 50));// ตั้งตำแหน่งของไวรัสให้เกิดที่ขอบขวาของหน้าจอและสุ่มตำแหน่งแนวตั้ง
			viruses.push_back(v);// เพิ่มไวรัสเข้าไปในเวกเตอร์
			virusClock.restart();// รีเซ็ตเวลาไวรัส
        }


		for (auto& v : viruses) {// สำหรับแต่ละไวรัสในเวกเตอร์ไวรัส
			v.sprite.move(-3.f, 0);// เคลื่อนที่ไวรัสไปทางซ้าย

			if (v.animClock.getElapsedTime().asMilliseconds() > 300) {// เช็คเวลาที่ผ่านไปตั้งแต่เปลี่ยนเฟรมมากกว่า 300 มิลลิวินาที
				v.sprite.setTexture(v.useFirstFrame ? virus2 : virus1);// สลับระหว่างรูปไวรัส 1 และ 2
				v.useFirstFrame = !v.useFirstFrame;// เปลี่ยนรูป
				v.animClock.restart();// รีเซ็ตเวลาอนิเมชั่น
            }
        }

		for (int i = 0; i < bullets.size(); i++) {// สำหรับแต่ละกระสุนในเวกเตอร์กระสุน
			for (int j = 0; j < viruses.size(); j++) {// สำหรับแต่ละไวรัสในเวกเตอร์ไวรัส
				if (bullets[i].shape.getGlobalBounds().intersects(viruses[j].sprite.getGlobalBounds())) {// ถ้ากระสุนชนกับไวรัส
					viruses.erase(viruses.begin() + j);// ลบไวรัสที่ถูกชนออก
					bullets.erase(bullets.begin() + i);// ลบกระสุนที่ชนออก
					score += 10;// เพิ่มคะแนน 10 คะแนน
					break;// ออกจากลูป j เพื่อไม่ให้เกิดการลบหลายครั้งในรอบเดียว
                }
            }
        }

		shieldZone.setPosition(robot.getPosition().x + robot.getGlobalBounds().width + 10, 0);// ตั้งตำแหน่งของเขตห้ามไวรัสเข้ามา

		for (int i = 0; i < viruses.size(); i++) {// สำหรับแต่ละไวรัสในเวกเตอร์ไวรัส
			if (viruses[i].sprite.getGlobalBounds().intersects(shieldZone.getGlobalBounds())) {// เมื่อไวรัสชนกับเขตห้าม
				viruses.erase(viruses.begin() + i);// ลบไวรัสที่ชนออก
				hp--;// ลดจำนวน HP ของหุ่นยนต์
            }
        }

		if (hp <= 0) isGameOver = true;// ถ้า HP ของหุ่นยนต์น้อยกว่าหรือเท่ากับ 0 ให้เปลี่ยนสถานะเกมเป็นจบ

		if (animationClock.getElapsedTime().asMilliseconds() > 500) {// ถ้าเวลาที่ผ่านไปตั้งแต่เปลี่ยนเฟรมมากกว่า 500 มิลลิวินาที
			robot.setTexture(useFirstFrame ? robot2 : robot1);// สลับระหว่างรูปหุ่นยนต์ 1 และ 2
			useFirstFrame = !useFirstFrame;// เปลี่ยนรูป
			animationClock.restart();// รีเซ็ตเวลาอนิเมชั่น
        }

        if (isReloading) {// ถ้ากำลังรีโหลด
			float elapsed = reloadClock.getElapsedTime().asSeconds();// เช็คเวลาที่ผ่านไปตั้งแต่เริ่มรีโหลด
			if (elapsed >= 1.6f) {// ถ้าเวลาที่ผ่านไปมากกว่าหรือเท่ากับ 1.6 วินาที
				bulletsLeft = 12;// รีเซ็ตจำนวนกระสุนที่เหลือเป็น 12 นัด
				isReloading = false;// ตั้งค่าสถานะการรีโหลดเป็นเท็จ
				reloadText.setString("");// ล้างข้อความรีโหลด
            }
            else {
				int remaining = static_cast<int>(1.6f - elapsed + 1);// คำนวณเวลาที่เหลือในการรีโหลด
				reloadText.setString("Reloading... " + to_string(remaining));// แสดงข้อความรีโหลดพร้อมเวลาที่เหลือ
            }
        }


		scoreText.setString("Score: " + to_string(score) + "    HP: " + to_string(hp) + "    Bullets: " + to_string(bulletsLeft));// อัพเดทข้อความคะแนน, HP และจำนวนกระสุนที่เหลือ
    }

	void draw() {// ฟังก์ชันนี้จะวาดทุกอย่างที่อยู่ในเกม
		if (isGameOver) {// ถ้าเกมจบแล้ว
			window.draw(gameOverText);// แสดงผลความเกมจบ
            window.draw(scoreText);// แสดงผลคะแนน
            window.draw(backToMenuBtn);
            window.draw(backToMenuText);
            return;
        }

        window.draw(robot);// แสดงผลหุ่นยนต์
        window.draw(shieldZone);// แสดงผลเขตที่ห้ามไวรัสเข้า
		for (auto& b : bullets) window.draw(b.shape);// แสดงผลกระสุนทั้งหมด
		for (auto& v : viruses) window.draw(v.sprite);// แสดงผลไวรัสทั้งหมด
		window.draw(scoreText);// แสดงผลคะแนน
		window.draw(reloadText);// แสดงผลข้อความรีโหลด
    }
};

// ------------------------- Main -------------------------
int main() {
	srand(time(0));// ตั้งค่า seed สำหรับการสุ่มตัวเลข
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Robot vs Virus");// สร้างหน้าต่างเกม
	window.setFramerateLimit(60);// ตั้งค่าเฟรมเรตให้ไม่เกิน 60 เฟรมต่อวินาที

	sf::Font font;// สร้างออบเจ็กต์ฟอนต์
	if (!font.loadFromFile("FC Minimal Bold Italic.ttf")) {// โหลดฟอนต์จากไฟล์
		cerr << "Font load failed!" << endl;// ถ้าโหลดฟอนต์ไม่สำเร็จ
		return -1;// ออกจากโปรแกรม
    }

	MainMenu menu(font);// สร้างออบเจ็กต์เมนูหลัก
	GameManager game(window, font);// สร้างออบเจ็กต์เกมเมเนเจอร์
	GameState state = GameState::MENU;// กำหนดสถานะเริ่มต้นของเกมเป็นเมนู

	while (window.isOpen()) {// วนลูปจนกว่าหน้าต่างจะถูกปิด
		sf::Event event;// สร้างออบเจ็กต์เหตุการณ์
		while (window.pollEvent(event)) {// เช็คเหตุการณ์ที่เกิดขึ้น
			if (event.type == sf::Event::Closed)// ถ้าเหตุการณ์คือปิดหน้าต่าง
				window.close();// ปิดหน้าต่างเกม

			if (state == GameState::MENU && event.type == sf::Event::MouseButtonPressed) {// ถ้าอยู่ในเมนูและมีการกดปุ่มเมาส์
				if (menu.handleClick(sf::Mouse::getPosition(window))) {// ถ้าคลิกที่ปุ่ม Start
					state = GameState::PLAYING;// เปลี่ยนสถานะเกมเป็นเล่น
                }
            }

			if (state == GameState::PLAYING || state == GameState::GAME_OVER)// ถ้าอยู่ในสถานะเล่นหรือเกมจบ
				game.handleEvent(event, state);// ให้เกมเมเนเจอร์จัดการเหตุการณ์
        }

		window.clear();// ล้างหน้าต่างก่อนวาดใหม่

		if (state == GameState::MENU) {// ถ้าอยู่ในเมนู
			menu.draw(window);// วาดเมนู
        }
		else {// ถ้าอยู่ในสถานะเล่นหรือเกมจบ
			game.update(state);// อัพเดทสถานะของเกม
			game.draw();// วาดทุกอย่างในเกม
        }

		window.display();// แสดงผลหน้าต่างเกม
    }

	return 0;// คืนค่า 0 เพื่อบอกว่าโปรแกรมทำงานสำเร็จ
}
