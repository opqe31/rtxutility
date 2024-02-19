#pragma once

#include "../plugin_sdk/plugin_sdk.hpp"

class c_notify {
public:
	enum e_color_mode {
		custom_color,
		text,
		active_text
	};

	struct s_notify_text 
	{
		std::string text;
		std::string text_real;
		std::string alt_text;

		vector text_size = vector::zero;

		uint32_t color = 0;
		e_color_mode color_mode = e_color_mode::text;

		float timer_end = -1.f;
		float duration_end = -1.f;

		s_notify_text( std::string str, e_color_mode e_color_mode = e_color_mode::text, uint32_t u_color = 0, float fl_timer_end = -1.f, float fl_duration_entry = -1.f, std::string text_alternate = "" )
		{
			text = str;
			alt_text = text_alternate;

			color = u_color;
			color_mode = e_color_mode;

			timer_end = fl_timer_end;
			duration_end = fl_duration_entry;
		}

		auto expired_o_text( ) -> bool
		{
			return duration_end != -1.f && gametime->get_time( ) >= duration_end;
		}

		auto valid_timer( ) -> bool
		{
			return timer_end != -1.f && timer_end >= gametime->get_time( );
		}
	};

	void add_text( float duration, std::vector<s_notify_text> text );
	void add( uint32_t* icon_l, uint32_t* icon_r, float duration, std::vector<s_notify_text> text );

	struct s_notify_entry 
	{
		std::string total_text;
		std::vector<s_notify_text> text;

		vector text_size = vector::zero;
		vector pos = vector::zero;

		uint32_t* icon1 = nullptr;
		uint32_t* icon2 = nullptr;

		float time = 8.f;
		float time_end = 0.f;

		float alpha = 1.f;
		float t_last_alpha_anim = 0.f;

		float text_alpha = 0.001f;
		float t_last_text_alpha_anim = 0.f;

		vector v_delta = vector::zero;
		vector v_base_pos = vector::zero;
		float fl_delta = 0.001f;
		float t_last_delta_change = 0.f;

		float fl_step = 0.001f;
		float t_last_step_change = 0.f;

		float fl_y_adjustment = 0.005f;
		float t_step_removal_anim = 0.f;

		int font_sz = 0;
		int side = 0;

		s_notify_entry( std::vector<s_notify_text> str, float t_end, int i_side = 0, uint32_t* icon_1 = nullptr, uint32_t* icon_2 = nullptr )
		{ 
			text = str;

			icon1 = icon_1;
			icon2 = icon_2;

			time = t_end;

			t_last_alpha_anim = t_last_text_alpha_anim = t_last_delta_change = t_step_removal_anim = gametime->get_time( );
			side = i_side;
		}
	};

	void load( );
	void unload( );

	void draw( );
	void process_spell_cast( game_object_script sender, spell_instance_script spell );

	bool is_r_enabled( champion_id id );

	TreeEntry* e_notify_vote;

	TreeEntry* e_text_color_active;
	TreeEntry* e_text_color;
	TreeEntry* e_background_color;

	TreeEntry* e_sep_wl_r_allies;
	TreeEntry* e_notify_ally;
	TreeEntry* e_notify_enemy;
	TreeEntry* e_notify_jungle;
	TreeEntry* e_use_champion_names;
private:
	TreeEntry* e_notify_side;
	TreeEntry* e_text_size;
	TreeEntry* e_notify_duration;

	std::vector< s_notify_entry > m_notify;
	std::map<champion_id, TreeEntry*> m_r_whitelist;

	struct s_spell_track 
	{
		float r_cd_start;
		bool r_notified;
		float t_last_r_notify = -99.f;

		float s1_cd_start;
		bool s1_notified;
		float t_last_s1_notify = -99.f;

		float s2_cd_start;
		bool s2_notified;
		float t_last_s2_notify = -99.f;
	};

	std::map<uint32_t, s_spell_track> m_cd_notify;
	std::map<uint32_t, float> m_last_process_cast;
};

extern c_notify* g_notify;