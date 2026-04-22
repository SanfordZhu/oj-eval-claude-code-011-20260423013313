#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {

template<typename T, class Compare = std::less<T>>
class priority_queue {
	struct Node {
		T val;
		Node *left;
		Node *right;
		int dist;
		Node(const T &v) : val(v), left(nullptr), right(nullptr), dist(1) {}
	};
	struct PlanEntry {
		Node *node;
		Node *new_left;
		Node *new_right;
		int new_dist;
		PlanEntry *next;
		PlanEntry(Node *n, Node *l, Node *r, int d, PlanEntry *nx)
			: node(n), new_left(l), new_right(r), new_dist(d), next(nx) {}
	};
	struct MergeResult {
		Node *root;
		int dist;
		MergeResult(Node *r = nullptr, int d = 0) : root(r), dist(d) {}
	};

	Node *rt = nullptr;
	size_t sz = 0;
	Compare cmp;

	static int dist_of(Node *x) { return x ? x->dist : 0; }

	static void commit_plan(PlanEntry *p) {
		for (PlanEntry *cur = p; cur; cur = cur->next) {
			cur->node->left = cur->new_left;
			cur->node->right = cur->new_right;
			cur->node->dist = cur->new_dist;
		}
	}
	static void free_plan(PlanEntry *p) {
		while (p) {
			PlanEntry *n = p->next;
			delete p;
			p = n;
		}
	}

	MergeResult merge_plan(Node *a, Node *b, PlanEntry *&plan) {
		if (!a) return MergeResult(b, dist_of(b));
		if (!b) return MergeResult(a, dist_of(a));
		bool swapab = false;
		try { swapab = cmp(a->val, b->val); }
		catch (...) { throw runtime_error(); }
		if (swapab) { Node *t = a; a = b; b = t; }
		MergeResult mr = merge_plan(a->right, b, plan);
		Node *new_left = a->left;
		Node *new_right = mr.root;
		int ld = dist_of(new_left);
		int rd = mr.dist;
		if (ld < rd) { Node *t = new_left; new_left = new_right; new_right = t; int td = ld; ld = rd; rd = td; }
		int nd = 1 + (ld < rd ? ld : rd);
		plan = new PlanEntry(a, new_left, new_right, nd, plan);
		return MergeResult(a, nd);
	}

	static void clear(Node *x) {
		if (!x) return;
		clear(x->left);
		clear(x->right);
		delete x;
	}
	static Node *clone(Node *x) {
		if (!x) return nullptr;
		Node *n = new Node(x->val);
		n->left = clone(x->left);
		n->right = clone(x->right);
		n->dist = x->dist;
		return n;
	}

public:
	priority_queue() {}
	priority_queue(const priority_queue &other) {
		rt = clone(other.rt);
		sz = other.sz;
		cmp = other.cmp;
	}
	~priority_queue() { clear(rt); }

	priority_queue &operator=(const priority_queue &other) {
		if (this == &other) return *this;
		Node *newrt = clone(other.rt);
		size_t newsz = other.sz;
		clear(rt);
		rt = newrt;
		sz = newsz;
		cmp = other.cmp;
		return *this;
	}

	const T & top() const {
		if (empty()) throw container_is_empty();
		return rt->val;
	}

	void push(const T &e) {
		Node *n = new Node(e);
		PlanEntry *plan = nullptr;
		try {
			MergeResult mr = merge_plan(rt, n, plan);
			commit_plan(plan);
			free_plan(plan);
			rt = mr.root;
			++sz;
		} catch (...) {
			free_plan(plan);
			delete n;
			throw runtime_error();
		}
	}

	void pop() {
		if (empty()) throw container_is_empty();
		PlanEntry *plan = nullptr;
		Node *old = rt;
		try {
			MergeResult mr = merge_plan(rt->left, rt->right, plan);
			commit_plan(plan);
			free_plan(plan);
			rt = mr.root;
			delete old;
			--sz;
		} catch (...) {
			free_plan(plan);
			throw runtime_error();
		}
	}

	size_t size() const { return sz; }
	bool empty() const { return sz == 0; }

	void merge(priority_queue &other) {
		if (this == &other) return;
		PlanEntry *plan = nullptr;
		try {
			MergeResult mr = merge_plan(rt, other.rt, plan);
			commit_plan(plan);
			free_plan(plan);
			rt = mr.root;
			sz += other.sz;
			other.rt = nullptr;
			other.sz = 0;
		} catch (...) {
			free_plan(plan);
			throw runtime_error();
		}
	}
};

}

#endif
