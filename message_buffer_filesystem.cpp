#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include "manoj.h"


int logged_userid;
long int pickupbox[200];
#define DISK "disk.dat"
#define blocksize 140
#define users_location 52500
#define rawdata 54740


void user_menu(char *msg);
void landing_page(char *msg);
void self_category_menu(char *msg, long int cat_address);
void show_all_posts(long int cat_address);

struct user_block
{
	int userid;
	long int first_category_address;
	char name[50];
};

struct category_block	
{
	long int next_category_address;
	long int first_post_address;
	char name[50];
};

struct post_block
{
	int userid;
	long int next_post_address;
	long int first_reply_address;
	char post[128];
};

struct reply_block
{
	int userid;
	long int next_reply_address;
	char reply[128];
};

long int get_free_location()
{
	FILE *p = fopen(DISK, "rb+");
	char c;
	long int block = 0;
	while (block < 52500)
	{
		c = fgetc(p);
		if (c == 0 || c == -1)
			break;
		block++;
	}
	if (block == 52500)
	{
		fclose(p);
		return -1;
	}
	fseek(p, -1, SEEK_CUR);
	fputc('1', p);
	fclose(p);
	return rawdata + block*blocksize;
}

int get_block(long int address)
{
	address = address - rawdata;
	return address / blocksize;
}

char* get_username(int userid)
{
	FILE *p = fopen(DISK, "rb+");
	int i = 0;
	struct user_block user;
	char *username = (char*)calloc(50, sizeof(char));

	while (i < 16)
	{
		fseek(p, users_location + i * blocksize, SEEK_SET);
		memset(&user, '\0', sizeof(user));
		fread(&user, sizeof(user), 1, p);
		if (user.userid == userid)
		{
			fclose(p);
			memcpy(username, &user.name, sizeof(user.name));
			return username;
		}
		i++;
	}
	fclose(p);
	return NULL;
}

long int get_user_location(int userid)
{
	FILE *p = fopen(DISK, "rb+");
	int i = 0;
	struct user_block user;

	while (i < 16)
	{
		fseek(p, users_location + i * blocksize, SEEK_SET);
		memset(&user, '\0', sizeof(user));
		fread(&user, sizeof(user), 1, p);
		if (user.userid == userid)
		{
			fclose(p);
			return users_location + i * blocksize;
		}
		i++;
	}
	fclose(p);
	return -1;
}

void create_post(long int cat_address)
{
	FILE *p = fopen(DISK, "rb+");
	fseek(p, cat_address, SEEK_SET);
	struct category_block cat;
	fread(&cat, sizeof(cat), 1, p);
	printf("\n\nType the post and hit enter: ");
	struct post_block post, temp_post;
	fflush(stdin);
	scanf("%[^\n]s", post.post);
	post.first_reply_address = 0;
	post.next_post_address = 0;
	post.userid = logged_userid;
	long int temp_address, address;
	if (cat.first_post_address == 0)
	{
		fclose(p);
		temp_address = get_free_location();
		p = fopen(DISK, "rb+");
		cat.first_post_address = temp_address;
		fseek(p, cat_address, SEEK_SET);
		fwrite(&cat, sizeof(cat), 1, p);
		fseek(p, cat.first_post_address, SEEK_SET);
		fwrite(&post, sizeof(post), 1, p);
		fclose(p);
		return self_category_menu("POST CREATED SUCCESSFULLY", cat_address);
	}
	else
	{
		fseek(p, cat.first_post_address, SEEK_SET);

		while (1)
		{
			memset(&temp_post, '\0', sizeof(temp_post));
			address = ftell(p);
			fread(&temp_post, sizeof(temp_post), 1, p);
			if (temp_post.next_post_address == 0)
				break;
			fseek(p, temp_post.next_post_address, SEEK_SET);
		}
		
		fclose(p);
		temp_address = get_free_location();
		p = fopen(DISK, "rb+");
		temp_post.next_post_address = temp_address;
		fseek(p, address, SEEK_SET);
		fwrite(&temp_post, sizeof(temp_post), 1, p);
		fseek(p, temp_post.next_post_address, SEEK_SET);
		fwrite(&post, sizeof(post), 1, p);
		fclose(p);
		return self_category_menu("POST CREATED SUCCESSFULLY", cat_address);
	}
}

void show_complete_post(long int post_address, long int cat_address)
{
	system("cls");
	FILE *p = fopen(DISK, "rb+");
	fseek(p, post_address, SEEK_SET);
	struct post_block post;
	struct reply_block reply;
	long int address = 0, new_address;
	int index = 1;
	pickupbox[index] = ftell(p);
	index++;
	fread(&post, sizeof(post), 1, p);
	char *str = get_username(post.userid);
	printf("Post by %s: \n\t%s\n\n", str, post.post);

	if (post.first_reply_address != 0)
	{
		fseek(p, post.first_reply_address, SEEK_SET);
		printf("Replies: \n\n");
		
		while (1)
		{
			memset(&reply, '\0', sizeof(reply));
			address = ftell(p);
			pickupbox[index] = ftell(p);
			fread(&reply, sizeof(reply), 1, p);
			printf("\t(%d)  %s: %s\n\n", index - 1, get_username(reply.userid), reply.reply);
			if (reply.next_reply_address == 0)
				break;
			fseek(p, reply.next_reply_address, SEEK_SET);
			index++;
		}
	}

	fclose(p);

	printf("Operations: \n\n");
	printf("\t[1] Reply\n");
	printf("\t[2] Delete a reply\n\n");
	printf("\t[3] Go Back\n");
	printf("\t[4] Log out\n");
	printf("\t[5] Exit\n\n");
	printf("Select an operation to perform: ");
	fflush(stdin);
	char c = getchar();

	switch (c)
	{
	case '1':
	{
		printf("\n\nReply: ");
		struct reply_block myReply;
		fflush(stdin);
		scanf("%[^\n]s", myReply.reply);
		myReply.next_reply_address = 0;
		myReply.userid = logged_userid;
		new_address = get_free_location();
		p = fopen(DISK, "rb+");
		if (address == 0)
		{
			fseek(p, post_address, SEEK_SET);
			memset(&post, '\0', sizeof(post));
			fread(&post, sizeof(post), 1, p);
			post.first_reply_address = new_address;
			fseek(p, post_address, SEEK_SET);
			fwrite(&post, sizeof(post), 1, p);
		}
		else
		{
			fseek(p, address, SEEK_SET);
			memset(&reply, '\0', sizeof(reply));
			fread(&reply, sizeof(reply), 1, p);
			reply.next_reply_address = new_address;
			fseek(p, address, SEEK_SET);
			fwrite(&reply, sizeof(reply), 1, p);
		}
		fseek(p, new_address, SEEK_SET);
		fwrite(&myReply, sizeof(myReply), 1, p);
		fclose(p);		
		return show_complete_post(post_address, cat_address);
		break;
	}
	case '2':
	{
		if (address == 0)
			return self_category_menu("ERROR: No replies yet...", cat_address);
		printf("\nSelect the reply to delete: ");
		int replynum;
		scanf("%d", &replynum);
		if (replynum < 1 || replynum > index)
			return self_category_menu("ERROR: Wrong option selected", cat_address);
		p = fopen(DISK, "rb+");
		fseek(p, pickupbox[replynum], SEEK_SET);

		if (replynum == 1)
		{
			memset(&post, '\0', sizeof(post));
			fread(&post, sizeof(post), 1, p);
			fseek(p, get_block(post.first_reply_address), SEEK_SET);
			fputc('\0', p);
			fseek(p, post.first_reply_address, SEEK_SET);
			memset(&reply, '\0', sizeof(reply));
			fread(&reply, sizeof(reply), 1, p);
			fseek(p, post.first_reply_address, SEEK_SET);
			post.first_reply_address = reply.next_reply_address;
			memset(&reply, '\0', sizeof(reply));
			fwrite(&reply, sizeof(reply), 1, p);
			fseek(p, post_address, SEEK_SET);
			fwrite(&post, sizeof(post), 1, p);
			fclose(p);
			return show_complete_post(post_address, cat_address);
		}
		
		struct reply_block todelete, prev;

		memset(&prev, '\0', sizeof(prev));
		fread(&prev, sizeof(prev), 1, p);
		fseek(p, get_block(prev.next_reply_address), SEEK_SET);
		fputc('\0', p);
		fseek(p, pickupbox[replynum], SEEK_SET);
		fread(&prev, sizeof(prev), 1, p);
		fseek(p, prev.next_reply_address, SEEK_SET);
		fread(&todelete, sizeof(todelete), 1, p);
		fseek(p, prev.next_reply_address, SEEK_SET);
		prev.next_reply_address = todelete.next_reply_address;
		memset(&todelete, '\0', sizeof(todelete));
		fwrite(&todelete, sizeof(todelete), 1, p);
		fseek(p, pickupbox[replynum], SEEK_SET);
		fwrite(&prev, sizeof(prev), 1, p);
		fclose(p);
		return show_complete_post(post_address, cat_address);
		break;
	}
	case '3':
		return show_all_posts(cat_address);
	case '4':
		logged_userid = -1;
		return landing_page("LOGGED OUT");
	case '5':
		system("exit");
		break;
	default:
		return self_category_menu("Error: Wrong option choosen", cat_address);
	}
	

}

void show_all_posts(long int cat_address)
{
	FILE *p = fopen(DISK, "rb+");
	fseek(p, cat_address, SEEK_SET);
	struct category_block cat;
	int i = 0, index = 1;
	fread(&cat, sizeof(cat), 1, p);
	if (cat.first_post_address == 0)
	{
		fclose(p);
		return self_category_menu("No posts created yet...", cat_address);
	}
	else
	{
		fseek(p, cat.first_post_address, SEEK_SET);
		struct post_block post;
		system("cls");
		printf("List of posts\n\n");
		
		while (1)
		{
			memset(&post, '\0', sizeof(post));
			pickupbox[index] = ftell(p);
			fread(&post, sizeof(post), 1, p);
			i = 0;
			printf("[%d] ", index);
			if (string_length(post.post) > 15){
				while (i < 15){
					putchar(post.post[i]);
					i++;
				}
				printf(".....\n");
			}
			else
				printf("%s\n", post.post);
			index += 1;
			if (post.next_post_address == 0)
				break;
			fseek(p, post.next_post_address, SEEK_SET);
		}
	}

	printf("\n\nSelect an option from below:\n\n");
	printf("[1] Select a post to see complete post and its replies\n");
	printf("[2] Go back\n");
	printf("[3] Log out\n");
	printf("[4] Exit\n");
	printf("\n\nEnter your option: ");
	fflush(stdin);
	char c = getchar();
	switch (c)
	{
	case '1':
	{
		printf("\n\nEnter the number shown in braces of the post: ");
		int option;
		scanf("%d", &option);
		if (option < 1 || option > index)
			return self_category_menu("ERROR: Wrong option choosen\n", cat_address);
		return show_complete_post(pickupbox[option], cat_address);
		break;
	}
	case '2':
		return self_category_menu("WELCOME", cat_address);
	case '3':
		logged_userid = -1;
		return landing_page("LOGGED OUT");
	case '4':
		system("exit");
		break;
	default:
		return self_category_menu("Error: Wrong option choosen", cat_address);
	}
}

void self_category_menu(char *msg, long int cat_address)
{
	system("cls");

	if (msg != NULL)
		printf("%s\n\n", msg);
	
	printf("[1] Create a post\n");
	printf("[2] Show all posts\n");
	printf("[3] Go back\n");
	printf("[4] Log out\n");
	printf("[5] Exit");
	printf("\n\nSelect an option: ");
	fflush(stdin);
	char c = getchar();
	switch (c)
	{
	case '1':
		return create_post(cat_address);
	case '2':
		return show_all_posts(cat_address);
	case '3': 
		return user_menu("WELCOME");
	case '4':
		logged_userid = -1;
		return landing_page("LOGGED OUT");
	case '5':
		system("exit");
		break;
	default:
		return self_category_menu("Error: Wrong option choosen", cat_address);
	}
}

void show_categories(int userid, int *index)
{
	long int address = get_user_location(userid);
	FILE *p = fopen(DISK, "rb+");
	fseek(p, address, SEEK_SET);

	struct user_block user;
	fread(&user, sizeof(user), 1, p);
	printf("\n\nCategories of %s: \n", user.name);
	if (user.first_category_address == 0)
	{
		fclose(p);
		return;
	}
	else
		fseek(p, user.first_category_address, SEEK_SET);

	struct category_block cat;
	int i = 0;
	
	while (1)
	{
		memset(&cat, '\0', sizeof(cat));
		fread(&cat, sizeof(cat), 1, p);
		printf("\t[%d] %s\n", *index + i, cat.name);
		pickupbox[*index] = ftell(p) - sizeof(cat);
		*index = *index + 1;
		if (cat.next_category_address != 0)
			fseek(p, cat.next_category_address, SEEK_SET);
		else
			break;		
	}

	fclose(p);
	return;
}

void create_category()
{
	struct category_block cat, temp;
	printf("\n\nEnter new category name: ");
	fflush(stdin);
	scanf("%[^\n]s", cat.name);
	cat.first_post_address = 0;
	cat.next_category_address = 0;
	long int block, address = get_user_location(logged_userid);
	FILE *p = fopen(DISK, "rb+");
	fseek(p, address, SEEK_SET);
	struct user_block user;
	fread(&user, sizeof(user), 1, p);
	if (user.first_category_address == 0)
	{
		fclose(p);
		block = get_free_location();
		p = fopen(DISK, "rb+");
		user.first_category_address = block;
		fseek(p, address, SEEK_SET);
		fwrite(&user, sizeof(user), 1, p);
		fseek(p, block, SEEK_SET);
		fwrite(&cat, sizeof(cat), 1, p);
		fclose(p);
		return user_menu("Category Created");
	}
	else
	{
		fseek(p, user.first_category_address, SEEK_SET);
		while (1)
		{
			memset(&temp, '\0', sizeof(temp));
			address = ftell(p);
			fread(&temp, sizeof(temp), 1, p);
			if (temp.next_category_address == 0)
				break;
			else
				fseek(p, temp.next_category_address, SEEK_SET);
		}
		fclose(p);
		block = get_free_location();
		p = fopen(DISK, "rb+");
		temp.next_category_address = block;
		fseek(p, address, SEEK_SET);
		fwrite(&temp, sizeof(temp), 1, p);
		fseek(p, block, SEEK_SET);
		fwrite(&cat, sizeof(cat), 1, p);
		fclose(p);
		return user_menu("Category Created");

	}
}

void user_menu(char *msg)
{
	system("cls");

	if (msg != NULL)
		printf("%s\n\n", msg);

	printf("[1] Show my categories\n");
	printf("[2] Show all categories\n");
	printf("[3] Create new category\n");
	printf("[4] Log out\n");
	printf("[5] Exit\n");
	printf("\n\nSelect an option: ");
	fflush(stdin);
	char c = getchar();
	switch (c)
	{
	case '1':
	{
		system("cls");
		printf("List of your categories");
		int index = 1;
		show_categories(logged_userid, &index);
		if (index == 1)
			return user_menu("No categories Created yet...");
		else
		{
			int option;
			printf("\n\nSelect a category: ");
			scanf("%d", &option);
			if (option >= 1 && option <= index)
				return self_category_menu("WELCOME", pickupbox[option]);
			else
				return user_menu("ERROR: Wrong option choosen");
		}
		break;
	}
	case '2':
	{
		int i, address, index = 1;
		struct user_block user;
		system("cls");
		printf("List of all categories");
		FILE *p = fopen(DISK, "rb+");
		for (i = 0; i < 16; i++)
		{
			fseek(p, users_location + i * blocksize, SEEK_SET);
			memset(&user, '\0', sizeof(user));
			fread(&user, sizeof(user), 1, p);
			if (user.userid == i + 1)
				show_categories(i + 1, &index);
		}
		fclose(p);
		printf("\n\nSelect a category: ");
		int option;
		scanf("%d", &option);
		if (option >= 1 && option <= index)
			return self_category_menu("WELCOME", pickupbox[option]);
		else
			return user_menu("ERROR: Wrong option choosen");		
		break;
	}
	case '3':
		create_category();
		break;
	case '4':
		logged_userid = -1;
		landing_page("Logged out");
		break;
	case '5':
		system("exit");
		break;
	default:
		user_menu("Error: Wrong option Choosen");
		break;
	}
	
}

void create_user()
{
	struct user_block user, temp;
	printf("\n\nEnter new username: ");
	fflush(stdin);
	scanf("%[^\n]s", user.name);
	FILE *p = fopen(DISK, "rb+");

	int i = 0;
	while (i < 16)
	{
		fseek(p, users_location + i * blocksize, SEEK_SET);
		memset(&temp, '\0', sizeof(temp));
		fread(&temp, sizeof(temp), 1, p);
		if (temp.userid != i + 1)
		{
			fseek(p, users_location + i * blocksize, SEEK_SET);
			user.userid = i + 1;
			user.first_category_address = 0;
			fwrite(&user, sizeof(user), 1, p);
			fclose(p);
			return landing_page("USER CREATED SUCCESSFULLY");
		}
		i++;
	}
	fclose(p);
	return landing_page("SORRY !! CANNOT ACCOMMODATE NEW USERS");

}

void user_login()
{
	struct user_block user;
	char username[50];
	printf("\n\nEnter username: ");
	fflush(stdin);
	scanf("%[^\n]s", username);
	FILE *p = fopen(DISK, "rb+");

	int i = 0;
	while (i < 16)
	{
		fseek(p, users_location + i * blocksize, SEEK_SET);
		memset(&user, '\0', sizeof(user));
		fread(&user, sizeof(user), 1, p);
		if (string_compare(user.name, username))
		{
			logged_userid = i + 1;
			fclose(p);
			return user_menu("WELCOME");
		}
		i++;
	}
	fclose(p);
	return landing_page("USER NOT FOUND");
}

void landing_page(char *msg)
{
	system("cls");

	if (msg != NULL)
		printf("%s\n\n", msg);
	printf("[1] Create User\n");
	printf("[2] User login\n");
	printf("[3] Exit\n");
	printf("\n\nEnter your option: ");
	fflush(stdin);
	char c = getchar();
	switch (c)
	{
	case '1':
	{
		create_user();
		break;
	}
	case '2':
		user_login();
		break;
	case '3':
		system("exit");
		break;
	default:
		landing_page("Error: Wrong option was choosen");
		break;
	}
}

int main()
{
	landing_page("WELCOME");
	return 0;
}

