#include <DxLib.h>
#include <stdexcept>
#include <vector>
#include <string>
#include <deque>
#include <map>
#include <set>
#include <initializer_list>
#include <random>
#include <format>
#include <functional>

constexpr int DAMAGE_DURATION = 60;

class effect
{
public:
	int id;
	std::string lore;
	std::vector<int> value;

	effect(int id, std::string lore, std::initializer_list<int> value) :id(id), lore(lore), value(value) {}

	bool operator<(const effect& t)const noexcept { return id < t.id; }
};

class card
{
public:
	int id;
	int graph;
	std::string name;
	effect ef;

	card(int id, std::string name, effect ef) :id(id), graph(-1), name(name), ef(ef) {}

	bool operator<(const card& t)const noexcept { return id < t.id; }
};

class duelist
{
public:
	int life;
	std::map<std::string, int> state;

	duelist(int life) :life(life) {}
};

class popup
{
	struct text
	{
		std::string str;
		int color, px, py;
		std::function<int(int)> mx, my;
		int duration, count;
		void draw()const { DrawString(px + (mx ? mx(count) : 0), py + (my ? my(count) : 0), str.c_str(), color); }
		bool update() { return ++count > duration; }
		text(const char* str, int color, int px, int py, int duration, std::function<int(int)> mx, std::function<int(int)> my)
			:str(str), color(color), px(px), py(py), duration(duration), count(0), mx(mx), my(my) {}
		text(const char* str, int color, int px, int py, int duration)
			:str(str), color(color), px(px), py(py), duration(duration), count(0) {}
		text(const std::string& str, int color, int px, int py, int duration, std::function<int(int)> mx, std::function<int(int)> my)
			:str(str), color(color), px(px), py(py), duration(duration), count(0), mx(mx), my(my) {}
		text(const std::string& str, int color, int px, int py, int duration)
			:str(str), color(color), px(px), py(py), duration(duration), count(0) {}
	};
	static inline std::deque<text> textlist;

public:
	static void push(const char* str, int color, int px, int py, int duration, std::function<int(int)> mx, std::function<int(int)> my)
	{
		textlist.emplace_back(str, color, px, py, duration, mx, my);
	}
	static void push(const char* str, int color, int px, int py, int duration)
	{
		textlist.emplace_back(str, color, px, py, duration);
	}
	static void push(const std::string& str, int color, int px, int py, int duration, std::function<int(int)> mx, std::function<int(int)> my)
	{
		textlist.emplace_back(str, color, px, py, duration, mx, my);
	}
	static void push(const std::string& str, int color, int px, int py, int duration)
	{
		textlist.emplace_back(str, color, px, py, duration);
	}
	static void update()
	{
		for (auto i = textlist.begin(); i != textlist.end();)
		{
			if (i->update())
			{
				i = textlist.erase(i);
				continue;
			}
			++i;
		}
	}
	static void draw()
	{
		for (const auto& i : textlist)
			i.draw();
	}
};

// �ŏI�I�ɂ��܂����ƃR���{�ł���Δn���݂����Ȑ������݂��銴�����ӎ�
// �Q�[���o�����X�͓�̎�
// �u�g�p�v�Ɓu�����v���ʕ��ɂȂ��Ă��܂��A�����_�Ŋ��ɃN�\�߂�ǂ������Ȃ��Ă��܂���
// ���ʂƃJ�[�h���ʕ��ɂȂ��Ă��܂����@�I��肾
// �����b�N�͎�������̂����邢�̂ł��Ȃ�
// �����������Ƃ������n�߂�Ƃ��Ⴀ��ԕω������邢�ȁ@������
// ���ʂ̂Ȃ��A�P�ɐςݏグ�邾���̂�Ȃ炢���񂾂���
//

// �����ɂ���
// ���Ɏg�p����J�[�h�n�ɂ���
// ���������J�[�h/���ʂ��X�^�b�N�`���Őςݏグ��@�������͒��O�̂��̂����ۑ�����@���ʂƃJ�[�h�͓������ƁA���ƂŃe�L�X�g����
// ���O�̃J�[�h�����āA���ꂪ�u���Ɏg�p����J�[�h�v�n�������ꍇ���ʂ�K�p
// ���ʂ͒l�𒼂Ŏ��@�Q�Ƃł͂Ȃ�
//

// ���߃L��������
// �����̑S�Ă��o�O�̉���
// ����ł��Ď����I�Ȍ��ʂ̓`���[�W�Ɠ����Ƃ����@�~���悤���Ȃ�
// ����������Ȃ��Ă������񂶂�ˁH
// ����@�{�X���Ƃ̑����͊m���ɂ����񂾂���
//

std::mt19937 mt(std::random_device{}());

const std::vector<effect> effectlist =
{
	{0,"�����{}�̃_���[�W",{10}},
	{1,"�����{}�̃_���[�W+�g�p�̓x�ɗ^�_���[�W{}�{",{4,2}},
	{2,"����ɓł�{}�t�^",{8}},
	{1000,"�O�ɔ����������ʂƓ������ʂ𔭓�����",{}},
	{4,"����̏�ԕω���{}�{���A�����ɓ�����ԕω���t�^����",{3}},
	{5,"�����Ƒ���̏�ԕω������ׂď������A���v�ʂ̃_���[�W�𑊎�ɗ^����",{}},
	{6,"���̃J�[�h�ȊO�̑S�ẴJ�[�h�̒����烉���_������{}��",{3}},
	{100,"���ɔ���������ʂɋL�ڂ��ꂽ������{}�{����",{2}},
	//{101,"���Ɏg�p����J�[�h��{0}�񔭓�����\n�������̃J�[�h�������ꍇ�A����Ɏ��̃J�[�h����*{0}�񔭓�����",{2}},
	// �h��n�̔\�͂������Ă�����
	// ���������G�������Ȃ��̂Ȃ�Ƃ���������
	{3,"����ɉ����{0}�t�^{1}��",{2,3}},
};

std::vector<card> cardlist =
{
	{0,"��������",effectlist[0]},
	{1,"�{�X��",effectlist[1]},
	{2,"�|�W��",effectlist[2]},
	{3,"�܂˂���",effectlist[3]},
	{4,"����̎�",effectlist[4]},
	{5,"�N�I�U��",effectlist[5]},
	{6,"��т��ӂ�",effectlist[6]},
	{7,"�`���[�W",effectlist[7]},
	//{8,"����",effectlist[8]},
	{9,"�u���C�Y",effectlist[8]},
};

duelist player(100);
duelist enemy(100000);

std::deque<effect> stack;

void action(effect e)
{
	if (!stack.empty() && e.id != 1000)
	{
		switch (stack.front().id - 100)
		{
		case 0:
			// ���ɔ���������ʂɋL�ڂ��ꂽ������n�{����
			for (auto& i : e.value)
				i *= stack.front().value[0];
			break;
			//case 1:
			//	// ���Ɏg�p����J�[�h��n�񔭓����� �������̃J�[�h�������ꍇ�A����Ɏ��̃J�[�h����*n�񔭓�����
			//	if (e.id != stack.front().id)
			//		for (int i = stack.front().value[0]; i > 1; --i)
			//			action(e);
			//	else
			//		e.value[0] *= stack.front().value[0];
			//	break;
		}
	}
	switch (e.id)
	{
	case 0:
		// �_���[�Wn
		enemy.life -= e.value[0];
		popup::push(std::to_string(e.value[0]), 0xffffff, 320 + std::uniform_int_distribution<>{-15, 15}(mt), 160 + std::uniform_int_distribution<>{-15, 15}(mt), DAMAGE_DURATION, [](int) {return 0; }, [](int t) {return t / 4; });
		break;
	case 1:
	{
		// �_���[�Wn+�g�p�̓x�ɍU����n�{
		auto baibai = player.state.find("�{�X��");
		if (baibai == player.state.end())
		{
			baibai = player.state.emplace("�{�X��", 1).first;
		}
		enemy.life -= e.value[0] * baibai->second;
		popup::push(std::to_string(e.value[0] * baibai->second), 0xffffff, 320 + std::uniform_int_distribution<>{-15, 15}(mt), 160 + std::uniform_int_distribution<>{-15, 15}(mt), DAMAGE_DURATION, [](int) {return 0; }, [](int t) {return t / 4; });
		baibai->second *= e.value[1];
	}
	break;
	case 2:
		// ��n
		enemy.state["��"] += e.value[0];
		popup::push(std::format("��+{}", e.value[0]), 0xffffff, 320 + std::uniform_int_distribution<>{-15, 15}(mt), 160 + std::uniform_int_distribution<>{-15, 15}(mt), DAMAGE_DURATION, [](int) {return 0; }, [](int t) {return t / 4; });
		break;
	case 3:
		// [�G/n��]����(xn)
		for (int i = 0; i < e.value[1]; ++i)
		{
			enemy.state["����"] += e.value[0];
			popup::push(std::format("����+{}", e.value[0]), 0xffffff, 320 + std::uniform_int_distribution<>{-15, 15}(mt), 160 + std::uniform_int_distribution<>{-15, 15}(mt), DAMAGE_DURATION, [](int) {return 0; }, [](int t) {return t / 4; });
		}
		break;
	case 4:
		// ����̏�ԕω���n�{���A�����ɓ�����ԕω���t�^����
		for (auto& i : enemy.state)
		{
			player.state[i.first] += i.second *= e.value[0];
			popup::push(std::format("{}={}", i.first, i.second), 0xffffff, 320 + std::uniform_int_distribution<>{-15, 15}(mt), 160 + std::uniform_int_distribution<>{-15, 15}(mt), DAMAGE_DURATION, [](int) {return 0; }, [](int t) {return t / 4; });
		}
		break;
	case 5:
	{
		// �����Ƒ���̏�ԕω������ׂď������A���v�ʂ̃_���[�W�𑊎�ɗ^����
		int statenum = 0;
		for (const auto& i : player.state)
			statenum += i.second;
		player.state.clear();
		for (const auto& i : enemy.state)
			statenum += i.second;
		enemy.state.clear();
		enemy.life -= statenum;
		popup::push(std::to_string(statenum), 0xffffff, 320 + std::uniform_int_distribution<>{-15, 15}(mt), 160 + std::uniform_int_distribution<>{-15, 15}(mt), DAMAGE_DURATION, [](int) {return 0; }, [](int t) {return t / 4; });
	}
	break;
	case 6:
		// ���̃J�[�h�ȊO�̃J�[�h�̒����烉���_������n��
		for (int i = 0; i < e.value[0];)
		{
			auto r = std::uniform_int_distribution<>{ 0, (int)cardlist.size() - 1 }(mt);
			if (r == 3)
				continue;
			action(cardlist[r].ef);
			++i;
		}
		break;
	}
	if (e.id == 1000)
	{
		// �O�ɔ����������ʂƓ������ʂ𔭓�����
		if (!stack.empty())
			action(stack.front());
	}
	else
	{
		stack.push_front(e);
	}
}

constexpr int cardsizex = 120;
constexpr int cardsizey = 180;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	SetGraphMode(640, 480, 32);
	ChangeWindowMode(TRUE);

	if (DxLib_Init() == -1)
		return -1;

	SetDrawScreen(DX_SCREEN_BACK);
	//SetFontSize(12);

	cardlist[0].graph = LoadGraph("data/kougeki.png");
	cardlist[1].graph = LoadGraph("data/bai.png");
	cardlist[2].graph = LoadGraph("data/posi.png");
	cardlist[3].graph = LoadGraph("data/mono.png");
	cardlist[4].graph = LoadGraph("data/ana.png");
	cardlist[5].graph = LoadGraph("data/sin.png");
	cardlist[6].graph = LoadGraph("data/yubi.png");
	cardlist[7].graph = LoadGraph("data/cha.png");

	std::vector<card> hand;

	int mousex, mousey;
	char mouse1 = 0;

	int select;

	while (ProcessMessage() == 0)
	{
		while (hand.size() < 5)
		{
			hand.push_back(cardlist[std::uniform_int_distribution<>{0, (int)cardlist.size() - 1}(mt)]);
		}

		GetMousePoint(&mousex, &mousey);
		mouse1 <<= 1;
		mouse1 += (GetMouseInput() & MOUSE_INPUT_1) != 0;
		if (mousey >= 300 && mousex >= 20 && mousex < 620)
			select = (mousex - 20) / cardsizex;
		else
			select = -1;
		if ((mouse1 & 0b00000011) == 1 && select != -1)
		{
			action(hand[select].ef);
			hand.erase(hand.begin() + select);
			
			//switch (std::uniform_int_distribution<>{0, 4}(mt))
			//{
			//case 0:
			//	player.life -= 5;
			//	break;
			//}
		}
		popup::update();

		ClearDrawScreen();

		DrawString(320, 5, std::format("�����Life:{}", enemy.life).c_str(), 0xffffff);

		for (int i = 0; i < __min(stack.size(), 5); ++i)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255 - i * 48);
			if (stack[i].value.size() == 0)
				DrawString(5, 170 + 20 * i, stack[i].lore.c_str(), 0xffffff);
			else if (stack[i].value.size() == 1)
				DrawString(5, 170 + 20 * i, std::vformat(stack[i].lore, std::make_format_args(stack[i].value[0])).c_str(), 0xffffff);
			else if (stack[i].value.size() == 2)
				DrawString(5, 170 + 20 * i, std::vformat(stack[i].lore, std::make_format_args(stack[i].value[0], stack[i].value[1])).c_str(), 0xffffff);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		for (int i = 0; i < hand.size(); ++i)
		{
			DrawGraph(20 + i * cardsizex, 300, hand[i].graph, FALSE);
			DrawBox(20 + i * cardsizex, 300, 20 + (i + 1) * cardsizex, 300 + cardsizey, 0xdddddd, FALSE);
			DrawString(22 + i * cardsizex, 302, hand[i].name.c_str(), 0xffffff);
			if (i == select)
			{
				SetDrawBlendMode(DX_BLENDMODE_ADD, 64);
				DrawBox(20 + i * cardsizex, 300, 20 + (i + 1) * cardsizex, 300 + cardsizey, 0xffffff, TRUE);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
				if (hand[i].ef.value.size() == 0)
					DrawString(22, 280, hand[i].ef.lore.c_str(), 0xffffff);
				else if (hand[i].ef.value.size() == 1)
					DrawString(22, 280, std::vformat(hand[i].ef.lore, std::make_format_args(hand[i].ef.value[0])).c_str(), 0xffffff);
				else if (hand[i].ef.value.size() == 2)
					DrawString(22, 280, std::vformat(hand[i].ef.lore, std::make_format_args(hand[i].ef.value[0], hand[i].ef.value[1])).c_str(), 0xffffff);
			}
		}
		popup::draw();

		ScreenFlip();
		clsDx();
	}

	DxLib_End();
	return 0;
}

